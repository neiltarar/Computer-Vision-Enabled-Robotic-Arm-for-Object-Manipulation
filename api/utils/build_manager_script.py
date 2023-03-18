from pathlib import Path
from string import Template
from utils.mediapipe_utils import find_isp_scale_params


def build_manager_script():
    SCRIPT_DIR = Path(__file__).resolve().parent
    TEMPLATE_MANAGER_SCRIPT_SOLO = str(SCRIPT_DIR / "template_manager_script_solo.py")
    TEMPLATE_MANAGER_SCRIPT_DUO = str(SCRIPT_DIR / "template_manager_script_duo.py")
    '''
    The code of the scripting node 'manager_script' depends on :
        - the score threshold,
        - the video frame shape
    So we build this code from the content of the file template_manager_script_*.py which is a python template
    '''
    resolution = (1920, 1080)
    internal_frame_height = 640
    width, scale_nd = find_isp_scale_params(
        internal_frame_height * resolution[0] / resolution[1], resolution, is_height=False)
    internal_fps = 29
    img_h = int(round(resolution[1] * scale_nd[0] / scale_nd[1]))
    img_w = int(round(resolution[0] * scale_nd[0] / scale_nd[1]))
    pad_h = (img_w - img_h) // 2
    pad_w = 0
    frame_size = img_w
    solo = False
    trace = 0
    pd_score_thresh = 0.5
    lm_score_thresh = 0.5

    crop_w = 0
    xyz = True
    use_handedness_average = True
    single_hand_tolerance_thresh = 10
    use_same_image = True
    use_world_landmarks = False

    # Read the template
    with open(TEMPLATE_MANAGER_SCRIPT_SOLO if solo else TEMPLATE_MANAGER_SCRIPT_DUO, 'r') as file:
        template = Template(file.read())

    # Perform the substitution
    code = template.substitute(
        _TRACE1="node.warn" if trace & 1 else "#",
        _TRACE2="node.warn" if trace & 2 else "#",
        _pd_score_thresh=pd_score_thresh,
        _lm_score_thresh=lm_score_thresh,
        _pad_h=pad_h,
        _img_h=img_h,
        _img_w=img_w,
        _frame_size=frame_size,
        _crop_w=crop_w,
        _IF_XYZ="" if xyz else '"""',
        _IF_USE_HANDEDNESS_AVERAGE="" if use_handedness_average else '"""',
        _single_hand_tolerance_thresh=single_hand_tolerance_thresh,
        _IF_USE_SAME_IMAGE="" if use_same_image else '"""',
        _IF_USE_WORLD_LANDMARKS="" if use_world_landmarks else '"""',
    )
    # Remove comments and empty lines
    import re
    code = re.sub(r'"{3}.*?"{3}', '', code, flags=re.DOTALL)
    code = re.sub(r'#.*', '', code)
    code = re.sub('\n\s*\n', '\n', code)
    # For debugging
    if trace & 8:
        with open("tmp_code.py", "w") as file:
            file.write(code)

    return code
