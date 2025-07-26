import glex as g
import gscript as gs
import math
import colorsys

color_pass: g.RenderPass = None

class FrameResource:
	def __init__(self):
		width = g.width()
		height = g.height()
		self.color_texture = g.Texture(gs.IMAGE_FORMAT_RGBA16F, gs.IMAGE_USAGE_COLOR_ATTACHMENT | gs.IMAGE_USAGE_TRANSFER_SOURCE, gs.IMAGE_ASPECT_COLOR, width, height, 1)
		self.frame_buffer = g.FrameBuffer(color_pass, [self.color_texture], width, height)

	def close(self):
		self.frame_buffer.close()
		self.color_texture.close()

frame_resources: list[FrameResource] = []

def startup():
	global color_pass, frame_resources
	color_attachment = g.AttachmentInfo(gs.IMAGE_FORMAT_RGBA16F, gs.IMAGE_STATE_UNDEFINED, gs.IMAGE_STATE_COLOR_ATTACHMENT, gs.IMAGE_STATE_COLOR_ATTACHMENT, gs.ATTACHMENT_USAGE_CLEAR_AND_WRITE, gs.ATTACHMENT_USAGE_DISCARD, 1)
	color_subpass = g.SubpassInfo([], [0], gs.ATTACHMENT_NONE, [])
	color_pass = g.RenderPass([color_attachment], [color_subpass], [])
	
	num_frames = g.num_render_ahead_frames()
	for i in range(num_frames):
		frame_resources.append(FrameResource())

def render():
	frame_idx = g.current_frame()
	frame_resource = frame_resources[frame_idx]
	time = g.time()
	hue = math.cos(0.0001 * time) * 0.5 + 0.5
	rgb = colorsys.hsv_to_rgb(hue, 0.6, 1.0)
	color_pass.begin(frame_resource.frame_buffer, [(*rgb, 1.0)])
	color_pass.end();
	return frame_resource.color_texture

def shutdown():
	for res in frame_resources:
		res.close()
	color_pass.close()