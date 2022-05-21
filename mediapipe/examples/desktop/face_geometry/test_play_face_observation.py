# importing pyglet module
import pyglet
import json
import pandas as pd
import matplotlib.pyplot as plt

# width of window
width = 720

# height of window
height = 1024

# caption i.e title of the window
title = "PlayActionTest"

# creating a window
window = pyglet.window.Window(width, height, title)

# video path
vidPath ="test_video.mp4"

# creating a media player object
player = pyglet.media.Player()

# creating a source object
source = pyglet.media.StreamingSource()

# load the media from the source
MediaLoad = pyglet.media.load(vidPath)

# add this media in the queue
player.queue(MediaLoad)

# play the video
player.play()

# on draw event
@window.event
def on_draw():

	# clear the window
	window.clear()

	# if player source exist
	# and video format exist
	if player.source and player.source.video_format:

		# get the texture of video and
		# make surface to display on the screen
		player.get_texture().blit(0, 0)


# key press event
@window.event
def on_key_press(symbol, modifier):

	# key "p" get press
	if symbol == pyglet.window.key.P:

		# pause the video
		player.pause()

		# printing message
		print("Video is paused")


	# key "r" get press
	if symbol == pyglet.window.key.R:

		# resume the video
		player.play()

		# printing message
		print("Video is resumed")

# seek video at time stamp = 4
#player.seek(0)

# pause the video
#player.pause()

with open('test_json.json') as data_file:
    data = json.load(data_file)
#df = pd.json_normalize(data['objects'][0]['actions'][0]['feeds'], 'tracked_positions')
df = pd.json_normalize(data['objects'][0]['actions'][0]['feeds'])

print(df)

fig = plt.figure()
st = fig.suptitle("angles", fontsize="x-large")

ax1 = fig.add_subplot(311)
ax1.plot(df['rotation.pitch'])
ax1.set_title("pitch")

ax2 = fig.add_subplot(312)
ax2.plot(df['rotation.yaw'])
ax2.set_title("yaw")

ax3 = fig.add_subplot(313)
ax3.plot(df['rotation.roll'])
ax3.set_title("roll")

fig.tight_layout()

# shift subplots down:
st.set_y(0.95)
fig.subplots_adjust(top=0.85)

plt.show(block=False)

# run the pyglet application
pyglet.app.run()
