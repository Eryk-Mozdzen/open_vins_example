# based on:
# https://docs.opencv.org/4.x/dc/dbb/tutorial_py_calibration.html

import numpy as np
import cv2 as cv
import glob
import sys

if len(sys.argv)!=4:
    print(f"USAGE:   python3 {sys.argv[0]} <dataset path> <chessboard grid size> <chessboard grid width [m]>\n")
    print(f"EXAMPLE: python3 {sys.argv[0]} datasets/calibration 8 0.02")
    sys.exit(-1)

DATASET_PATH = sys.argv[1]
GRID = int(sys.argv[2])
WIDTH = float(sys.argv[3])

images = glob.glob(DATASET_PATH + "/mav0/cam0/data/*.png")

criteria = (cv.TERM_CRITERIA_EPS + cv.TERM_CRITERIA_MAX_ITER, 30, 1e-6)

objp = np.zeros((1, (GRID - 1)*(GRID - 1), 3), np.float32)
objp[0, :, :2] = np.mgrid[0:(GRID - 1), 0:(GRID - 1)].T.reshape(-1, 2) * WIDTH

objpoints = []
imgpoints = []

for fname in images:
    img = cv.imread(fname)
    gray = cv.cvtColor(img, cv.COLOR_BGR2GRAY)
    ret, corners = cv.findChessboardCorners(gray, ((GRID - 1),(GRID - 1)), None)
    if ret:
        cv.cornerSubPix(gray, corners, (11,11), (-1,-1), criteria)
        objpoints.append(objp)
        imgpoints.append(corners)
        cv.drawChessboardCorners(img, ((GRID - 1),(GRID - 1)), corners, ret)
        cv.imshow("calibrate", img)
        cv.waitKey(1)

K = np.zeros((3,3))
D = np.zeros((4,1))
flags = cv.fisheye.CALIB_RECOMPUTE_EXTRINSIC + cv.fisheye.CALIB_CHECK_COND + cv.fisheye.CALIB_FIX_SKEW

rms, K, D, rvecs, tvecs = cv.fisheye.calibrate(
    objpoints, imgpoints, gray.shape[::-1], K, D, None, None,
    flags=flags, criteria=criteria
)

print(f"""%YAML:1.0

cam0:
  T_imu_cam:
    - [ ??? ??? ??? ??? ]
    - [ ??? ??? ??? ??? ]
    - [ ??? ??? ??? ??? ]
    - [   0   0   0   1 ]
  camera_model: pinhole
  distortion_coeffs: [{D[0, 0]}, {D[1, 0]}, {D[2, 0]}, {D[3, 0]}]
  distortion_model: equidistant
  intrinsics: [{K[0, 0]}, {K[1, 1]}, {K[0, 2]}, {K[1, 2]}]
  resolution: [{cv.imread(images[0]).shape[1]}, {cv.imread(images[0]).shape[0]}]
""")

cv.destroyAllWindows()
