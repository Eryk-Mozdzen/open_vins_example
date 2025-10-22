import numpy as np
import cv2 as cv
import glob

criteria = (cv.TERM_CRITERIA_EPS + cv.TERM_CRITERIA_MAX_ITER, 30, 1e-6)

objp = np.zeros((1, 7*7, 3), np.float32)
objp[0, :, :2] = np.mgrid[0:7, 0:7].T.reshape(-1, 2) * 0.02

objpoints = []
imgpoints = []

images = glob.glob('vio/datasets/calibration/mav0/cam0/data/*.png')

for fname in images:
    img = cv.imread(fname)
    gray = cv.cvtColor(img, cv.COLOR_BGR2GRAY)
    ret, corners = cv.findChessboardCorners(gray, (7,7), None)
    if ret:
        cv.cornerSubPix(gray, corners, (11,11), (-1,-1), criteria)
        objpoints.append(objp)
        imgpoints.append(corners)
        cv.drawChessboardCorners(img, (7,7), corners, ret)
        cv.imshow('img', img)
        cv.waitKey(1)

K = np.zeros((3,3))
D = np.zeros((4,1))
flags = cv.fisheye.CALIB_RECOMPUTE_EXTRINSIC + cv.fisheye.CALIB_CHECK_COND + cv.fisheye.CALIB_FIX_SKEW

rms, K, D, rvecs, tvecs = cv.fisheye.calibrate(
    objpoints, imgpoints, gray.shape[::-1], K, D, None, None,
    flags=flags, criteria=criteria
)

print(D)
print(K)

cv.destroyAllWindows()
