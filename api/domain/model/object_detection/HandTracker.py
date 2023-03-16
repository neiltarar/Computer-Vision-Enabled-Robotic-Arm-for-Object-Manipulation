import numpy as np
import depthai as dai
from pathlib import Path
from time import sleep


class HandTracker:
    def __init__(self) -> None:    
        self.SCRIPT_DIR = Path(__file__).resolve().parent
        print(self.SCRIPT_DIR, self.SCRIPT_DIR)
        pass
    

