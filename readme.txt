COMP9517 Computer Vision Report Implementation Details:

printf("usage: ./prog file_name.jpg\n");

Firstly I subdivided the picture in to 3 equal parts save as B G R respectively. To align the 3 pictures, I have implement 2 methods(SSD and normalized cross correlation ) to do exhausted search and doing data mining to produce final result.


SSD: I have implemented SSD in function SSD_IMP, and it picks a random window with size 1/20 pic width and 1/20 pic height within inner 64% of feature image(R or G), then do exhausted search towards a larger window in target image(B), it assign best match with highest value, and by tracking highest value, it returns transformation offset between feature image and target image.


NCC: I have used opencv library cvMatchTemplate to implement this method, similarly, I pick a small window in feature image and compare with larger window in target image, and do the match via cvMinMaxLoc.
Problem : Some times these 2 method cannot produce correct result, 1 fixed window some times could be in bad position (e.g. black holes in pic or similar confusing feature).


Solution : I use random sampling method to get higher alignment accuracy. I repeatedly sample 200 points from inner part of image, get the corresponding window based on the point, and compute SSD or NCC. The 200 result is stored, and the most frequent transformation value is more likely to be the correct transformation match. During experiment, I have noticed NCC have better accuracy than SSD, so I do NCC match 130 times and SSD 70 times.

To handle large images, I use image opencv pyramid to achieve speed up the matching. I use level 3 pyramid to get primary adjustment, and get and transformation value x, y correspondently . then I get transformation x= 4*x and transformation y= 4*y and apply the transformation update to original picture. Then the error range would be in -8 and +8 pixel for x and y, therefore , I conduct a smaller range search in original picture again to find correct transformation.

Additional stuff:
I did some smoothing and change light and contract just for experiment, these functions and implemented in void adjustments(IplImage* result) and the adjusted pic are saved separately in adj_result.jpg


