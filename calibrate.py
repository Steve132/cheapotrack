import cv2

img = cv2.imread(fname)
gray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)

objpoints = [] # float32
imgpoints = []
ret, mtx, dist, rvecs, tvecs = cv2.calibrateCamera(objpoints, imgpoints, gray.shape[::-1],None,None)

# Compute error
mean_error = 0
for i in xrange(len(objpoints)):
	imgpoints2, _ = cv2.projectPoints(objpoints[i], rvecs[i], tvecs[i], mtx, dist)
	error = cv2.norm(imgpoints[i], imgpoints2, cv2.NORM_L2) / len(imgpoints2)
	mean_error += error
mean_error /= len(objpoints)

print "mean error: ", mean_error
