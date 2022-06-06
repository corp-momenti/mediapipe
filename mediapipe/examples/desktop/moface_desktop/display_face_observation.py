import cv2
import json
import pandas as pd
import math

window_name = 'Display Observation'

width = 640

height = 480

#action index of observation
action_selection = 0

#reference file path
reference_path = "/Users/hoyounkim/Work/Momenti/Research/mediapipe/mediapipe/examples/desktop/moface_desktop/reference/5d9196df-110a-4ec6-bbbf-9e9090490453.png"

#observation file path
observation_path = "/Users/hoyounkim/Work/Momenti/Research/mediapipe/mediapipe/examples/desktop/moface_desktop/face-observation/b514221d-23ce-42b3-834e-d91f61055bba.json"


def display_roi(roi, image):
    color = (0, 255, 0)
    thickness = 3
    roi_x = int(math.ceil(roi['x'] * width))
    roi_y = int(math.ceil(roi['y'] * height))
    roi_width = int(math.ceil(roi['width'] * width))
    roi_height = int(math.ceil(roi['height'] * height))
    image = cv2.line(image, (roi_x, roi_y), (roi_x + roi_width, roi_y), color, thickness)
    image = cv2.line(image, (roi_x + roi_width, roi_y), (roi_x + roi_width, roi_y + roi_height), color, thickness)
    image = cv2.line(image, (roi_x + roi_width, roi_y + roi_height), (roi_x, roi_y + roi_height), color, thickness)
    image = cv2.line(image, (roi_x, roi_y + roi_height), (roi_x, roi_y), color, thickness)

def display_feeds(feeds, image):
    for feed in feeds:
        for point in feed['tracked_positions']:
            x = int(math.ceil(point[0] * width))
            y = int(math.ceil(point[1] * height))
            cv2.circle(image, (x, y), radius=1, color=(0, 0, 255), thickness=-1)

def display_action(selected_action, image):
    display_roi(selected_action['roi'], image)
    display_feeds(selected_action['feeds'], image)

#read reference
image = cv2.imread(reference_path)

#read face observation
with open(observation_path) as data_file:
    data = json.load(data_file)
df = pd.json_normalize(data['objects'][0]['actions'][action_selection]['feeds'])
print(df)

for idx, action in enumerate(data['objects'][0]['actions']):
    #if idx == 2:
        display_action(action, image)

# Displaying the image
cv2.imshow(window_name, image)
cv2.waitKey(0)
cv2.destroyAllWindows()
