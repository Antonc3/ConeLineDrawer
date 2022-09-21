# ConeLineDrawer

I first blurred the image to get remove some noise. I then used an hsv threshold to get the bright orange cones. I used the contour method in opencv to get the contours of the thershold image I had. I then filtered out the contours that were too small to be cones. I then got the midpoints of each of the contours, and used those midpoints to find lines of best fit. I did this by generating a line between every midpoint of each contour. I then approximated the cones into circles to check how many cones each line passed through. I found the two lines that passed through the most amount circles altogether, and drew those two lines.


![alt text](https://github.com/Antonc3/ConeLineDrawer/blob/main/answer.png))
