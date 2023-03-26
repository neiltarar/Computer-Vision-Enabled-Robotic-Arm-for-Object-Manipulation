import cv2
import numpy as np

LINES_HAND = [[0, 1], [1, 2], [2, 3], [3, 4],
              [0, 5], [5, 6], [6, 7], [7, 8],
              [5, 9], [9, 10], [10, 11], [11, 12],
              [9, 13], [13, 14], [14, 15], [15, 16],
              [13, 17], [17, 18], [18, 19], [19, 20], [0, 17]]


class DrawOnDetection:
    def __init__(self):
        self.lm_threshold = 0.4
        pass

    def draw_hand(self, hand, frame):
        # (info_ref_x, info_ref_y): coords in the image of a reference point
        # relatively to which hands information (score, handedness, xyz,...) are drawn
        info_ref_x = hand.landmarks[0, 0]
        info_ref_y = np.max(hand.landmarks[:, 1])

        # thick_coef is used to adapt the size of the draw landmarks features according to the size of the hand.
        thick_coef = hand.rect_w_a / 400
        if hand.lm_score > self.lm_threshold:

            cv2.polylines(frame, [np.array(hand.rect_points)], True, (0, 255, 255), 2, cv2.LINE_AA)

            lines = [np.array([hand.landmarks[point] for point in line]).astype(np.int32) for line in LINES_HAND]
            color = (255, 0, 0)
            cv2.polylines(frame, lines, False, color, int(1 + thick_coef * 3), cv2.LINE_AA)
            x0, y0 = info_ref_x - 40, info_ref_y + 40

            cv2.rectangle(frame, (x0, y0), (x0 + 100, y0 + 85), (220, 220, 240), -1)
            cv2.putText(frame, hand.label, (x0 + 50, y0 - 150), cv2.FONT_HERSHEY_PLAIN, 2,
                        (200, 255, 0), 2)
            cv2.putText(frame, f"X:{hand.xyz[0] / 10:3.0f} cm", (x0 + 10, y0 + 20), cv2.FONT_HERSHEY_PLAIN, 1,
                        (20, 180, 0), 2)
            cv2.putText(frame, f"Y:{hand.xyz[1] / 10:3.0f} cm", (x0 + 10, y0 + 45), cv2.FONT_HERSHEY_PLAIN, 1,
                        (255, 0, 0), 2)
            cv2.putText(frame, f"Z:{hand.xyz[2] / 10:3.0f} cm", (x0 + 10, y0 + 70), cv2.FONT_HERSHEY_PLAIN, 1,
                        (0, 0, 255), 2)

            # Show zone on which the spatial data were calculated
            cv2.rectangle(frame, tuple(hand.xyz_zone[0:2]), tuple(hand.xyz_zone[2:4]), (180, 0, 180), 2)
