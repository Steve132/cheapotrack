import cv2
import cv2.cv as cv
import numpy as np
import json

img_name = 'calibration/images/calibrationcross.jpg'
#img_name = 'calibration/images/glasses001.jpg'
#img_name = 'laserfrontcross.jpg'
img = cv2.imread(img_name,0)
window_img = cv2.resize(img, (int(img.shape[1] * (1080.0 / img.shape[0])), 1080))

centers = []
# mouse callback function
def draw_circle(event,x,y,flags,param):
	global centers
	if event == cv2.EVENT_LBUTTONDOWN:
#		cv2.circle(window_img,(x,y),20,(255,0,0),-1)
		centers.append((y,x))

# Display image, a window and bind the function to window
window_title = 'Click 4 circle centers'
cv2.namedWindow(window_title)
cv2.setMouseCallback(window_title,draw_circle)

while(len(centers) < 4):
	cv2.imshow(window_title,window_img)
	k = cv2.waitKey(20) & 0xFF
	if k == 27:
		break

window_img = cv2.medianBlur(window_img,5)
cimg = cv2.cvtColor(window_img,cv2.COLOR_GRAY2BGR)

circ_json = []
for c in range(len(centers)):
	pt = centers[c]
	# TODO: check bounds of pt
	max_radius = 100
	#   cv::Canny(gray, canny, 200,20);
	circles = cv2.HoughCircles(window_img[pt[0]-max_radius:pt[0]+max_radius, pt[1]-max_radius:pt[1]+max_radius],cv.CV_HOUGH_GRADIENT,1,20,param1=50,param2=30,minRadius=0,maxRadius=0)
	circles = np.uint16(np.around(circles))
	for i in circles[0,:]:
		if i[2] < max_radius:
			# draw the outer circle
			cv2.circle(cimg[pt[0]-max_radius:pt[0]+max_radius, pt[1]-max_radius:pt[1]+max_radius],(i[0],i[1]),i[2],(0,255,0),2)
			# draw the center of the circle
			cv2.circle(cimg[pt[0]-max_radius:pt[0]+max_radius, pt[1]-max_radius:pt[1]+max_radius],(i[0],i[1]),2,(0,0,255),3)
			print "adding to json:", i[0] + pt[0], i[1] + pt[1]
			circ_json.append({'x':float(i[0] + pt[0]), 'y':float(i[1] + pt[1]), 'radius':float(i[2])})
			# TODO: rescale from window image coords to original
			# draw the outer circle
			#pt_rescaled = [int(pt[0] / float(cimg.shape[0]) * float(img.shape[0])), int(pt[1] / float(cimg.shape[1]) * float(img.shape[1]))]
			#max_radius_rescaled = [int(max_radius / float(cimg.shape[0] * img.shape[0])), int(max_radius / float(cimg.shape[1] * img.shape[1]))]
			#i_rescaled = [int(i[0] / float(cimg.shape[0]) * float(img.shape[0])), int(i[1] / float(cimg.shape[1]) * float(img.shape[1]))]
			#print i_rescaled
			#cv2.circle(img[pt_rescaled[0]-max_radius_rescaled[0]:pt_rescaled[0]+max_radius_rescaled[0], pt_rescaled[1]-max_radius_rescaled[1]:pt_rescaled[1]+max_radius_rescaled[1]],(i_rescaled[0],i_rescaled[1]),i[2],(0,255,0),2)
			# draw the center of the circle
			#cv2.circle(img[pt_rescaled[0]-max_radius_rescaled[0]:pt_rescaled[0]+max_radius_rescaled[0], pt_rescaled[1]-max_radius_rescaled[1]:pt_rescaled[1]+max_radius_rescaled[1]],(i_rescaled[0],i_rescaled[1]),2,(0,0,255),3)

json.dump(circ_json, open("circles.json", 'w'))
cv2.imwrite(img_name + '_circles.jpg', cimg)
cv2.destroyAllWindows()
