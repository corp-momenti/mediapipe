# importing pyglet module
import pyglet
import json
import pandas as pd
import matplotlib.pyplot as plt
import time

play_feed_index = 0

action_selection = 0

# width of window
width = 640

# height of window
height = 480

# caption i.e title of the window
title = "PlayActionTest"

# creating a window
window = pyglet.window.Window(width, height, title)

label = pyglet.text.Label('',
                          font_name='Times New Roman',
                          font_size=36,
                          x=window.width//2, y=window.height//2,
                          anchor_x='center', anchor_y='center')

# observation path
observation_path = "/Users/hoyounkim/Work/Momenti/Research/mediapipe/mediapipe/examples/desktop/moface_desktop/face-observation/9c8827de-3516-45ac-80f3-2c10198e757d.json"

def display_rotation(action):
	df = pd.json_normalize(action['feeds'])
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

with open(observation_path) as data_file:
    data = json.load(data_file)
#df = pd.json_normalize(data['objects'][0]['actions'][0]['feeds'], 'tracked_positions')
#df = pd.json_normalize(data['objects'][0]['actions'][action_selection]['feeds'])

# video path
vidPath = data['file_path']

# creating a media player object
player = pyglet.media.Player()

# creating a source object
source = pyglet.media.StreamingSource()

# load the media from the source
MediaLoad = pyglet.media.load(vidPath)

# add this media in the queue
player.queue(MediaLoad)

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

	label.draw()


# key press event
@window.event
def on_key_press(symbol, modifier):
	global data
	global action_selection
	global play_feed_index
	global label

	num_of_actions = len(data['objects'][0]['actions'])
	# key "p" get press
	if symbol == pyglet.window.key.P:
		if play_feed_index < len(selected_action['feeds']) :
			# seek & pause the video
			print('seek at : ', selected_action['feeds'][play_feed_index]['timestamp'])
			player.seek(selected_action['feeds'][play_feed_index]['timestamp'])
			player.pause()
			# printing message
			play_feed_index = play_feed_index + 1
		print("Video is paused")
	if symbol == pyglet.window.key.F:
		action_selection = action_selection + 1
		if action_selection >= num_of_actions:
			action_selection = 0
		selected_action = data['objects'][0]['actions'][action_selection]
		play_feed_index = 0
		player.seek(selected_action['feeds'][play_feed_index]['timestamp'])
		player.pause()
		print(selected_action['desc'])
		label = pyglet.text.Label(selected_action['desc'],
				font_name='Times New Roman',
				font_size=36,
				x=window.width//2, y=window.height//2,
				anchor_x='center', anchor_y='center')
		display_rotation(selected_action)
	if symbol == pyglet.window.key.B:
		action_selection = action_selection - 1
		if action_selection < 0:
			action_selection = num_of_actions - 1
		selected_action = data['objects'][0]['actions'][action_selection]
		play_feed_index = 0
		player.seek(selected_action['feeds'][play_feed_index]['timestamp'])
		player.pause()
		print(selected_action['desc'])
		label = pyglet.text.Label(selected_action['desc'],
				font_name='Times New Roman',
				font_size=36,
				x=window.width//2, y=window.height//2,
				anchor_x='center', anchor_y='center')
		display_rotation(selected_action)
	if symbol == pyglet.window.key.C:
		window.close()
	# key "r" get press
	if symbol == pyglet.window.key.R:

		# resume the video
		player.play()

		# printing message
		print("Video is resumed")

#select the first action and show label
selected_action = data['objects'][0]['actions'][action_selection]
print(selected_action['desc'])
label = pyglet.text.Label(selected_action['desc'],
		font_name='Times New Roman',
		font_size=36,
		x=window.width//2, y=window.height//2,
		anchor_x='center', anchor_y='center')
#show the first frame of the first action
player.seek(selected_action['feeds'][play_feed_index]['timestamp'])
player.pause()

# run the pyglet application
pyglet.app.run()
