import glex
import glm

IMAGE_FORMAT_RGBA = 44
IMAGE_FORMAT_RGBA16F = 97
IMAGE_FORMAT_DEPTH16 = 124

IMAGE_USAGE_COLOR_ATTACHMENT = 0x10
IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT = 0x20
IMAGE_USAGE_TRANSIENT_ATTACHMENT = 0x40
IMAGE_USAGE_SAMPLED_TEXTURE = 0x04
IMAGE_USAGE_TRANSFER_SOURCE = 0X01
IMAGE_USAGE_TRANSFER_DEST = 0X02

IMAGE_STATE_UNDEFINED = 0
IMAGE_STATE_GENERAL = 1
IMAGE_STATE_COLOR_ATTACHMENT = 2
IMAGE_STATE_DEPTH_STENCIL_ATTACHMENT = 3
IMAGE_STATE_SHADER_READ = 5
IMAGE_STATE_TRANSFER_SOURCE = 6
IMAGE_STATE_TRANSFER_DEST = 7
IMAGE_STATE_PRESENT = 1000001002

IMAGE_ASPECT_COLOR = 0x00000001
IMAGE_ASPECT_DEPTH = 0x00000002

ATTACHMENT_USAGE_DISCARD = 0
ATTACHMENT_USAGE_DISCARD_AND_WRITE = 1
ATTACHMENT_USAGE_CLEAR_AND_WRITE = 2
ATTACHMENT_USAGE_READ_AND_WRITE = 3
ATTACHMENT_USAGE_READ_AND_KEEP = 4
ATTACHMENT_USAGE_READ_AND_DISCARD = 5

ATTACHMENT_EXTERNAL = 0xffffffff
ATTACHMENT_NONE = 0xffffffff

PIPELINE_STAGE_NONE = 0
PIPELINE_STAGE_ALL = 0x00010000
PIPELINE_STAGE_ALL_GRAPHICS = 0x00008000
PIPELINE_STAGE_FRAGMENT_SHADER = 0x00000080
PIPELINE_STAGE_COLOR_OUTPUT = 0x00000400
PIPELINE_STAGE_COPY = 0x100000000
PIPELINE_STAGE_BLIT = 0x400000000
PIPELINE_STAGE_CLEAR = 0x800000000

ACCESS_NONE = 0
ACCESS_READ = 0x00008000
ACCESS_TRANSFER_READ = 0x00000800
ACCESS_TRANSFER_WRITE = 0x00001000
ACCESS_UNIFORM_READ = 0x00000008
ACCESS_SHADER_SAMPLED_READ = 0x100000000
ACCESS_SHADER_STORAGE_READ = 0x200000000
ACCESS_COLOR_WRITE = 0x00000100

KEY_LMB = 0
KEY_RMB = 1
KEY_MMB = 2
KEY_ESCAPE = 256
KEY_F1 = 290
KEY_F2 = 291
KEY_F3 = 292
KEY_F4 = 293
KEY_F5 = 294
KEY_F6 = 295
KEY_F7 = 296
KEY_F8 = 297
KEY_F9 = 298
KEY_F10 = 299
KEY_F11 = 300
KEY_F12 = 301
KEY_Q = 81
KEY_W = 87
KEY_E = 69
KEY_R = 82
KEY_T = 84
KEY_Y = 89
KEY_U = 85
KEY_I = 73
KEY_O = 79
KEY_P = 80
KEY_A = 65
KEY_S = 83
KEY_D = 68
KEY_F = 70
KEY_G = 71
KEY_H = 72
KEY_J = 74
KEY_K = 75
KEY_L = 76
KEY_Z = 90
KEY_X = 88
KEY_C = 67
KEY_V = 86
KEY_B = 66
KEY_N = 78
KEY_M = 77
KEY_0 = 48
KEY_1 = 49
KEY_2 = 50
KEY_3 = 51
KEY_4 = 52
KEY_5 = 53
KEY_6 = 54
KEY_7 = 55
KEY_8 = 56
KEY_9 = 57
KEY_ACCENT = 96
KEY_MINUS = 45
KEY_EQUAL = 61
KEY_BACKSPACE = 259
KEY_LEFT_BRACKET = 91
KEY_RIGHT_BRACKET = 93
KEY_BACKSLASH = 92
KEY_SEMICOLON = 59
KEY_APOSTROPHE = 39
KEY_COMMA = 44
KEY_PERIOD = 46
KEY_SLASH = 47
KEY_TAB = 258
KEY_CAPSLOCK = 280
KEY_LEFT_SHIFT = 340
KEY_LEFT_CONTROL = 341
KEY_LEFT_WINDOWS = 343
KEY_LEFT_ALT = 342
KEY_RIGHT_SHIFT = 344
KEY_RIGHT_CONTROL = 345
KEY_MENU = 348
KEY_RIGHT_WINDOWS = 347
KEY_RIGHT_ALT = 346
KEY_SPACE = 32
KEY_RETURN = 257
KEY_PRINT_SCREEN = 283
KEY_SCROLL_LOCK = 281
KEY_PUSE_BREAK = 284
KEY_INSERT = 260
KEY_HOME = 268
KEY_PAGEUP = 266
KEY_DELETE = 261
KEY_END = 269
KEY_PAGE_DOWN = 267
KEY_UP = 265
KEY_DOWN = 264
KEY_LEFT = 263
KEY_RIGHT = 262
KEY_NUM_LOCK = 282
KEY_NUM_DIVIDE = 331
KEY_NUM_MULTIPLY = 332
KEY_NUM_MINUS = 333
KEY_NUM_ADD = 334
KEY_NUM_RETURN = 335
KEY_NUM_DOT = 330
KEY_NUM0 = 320
KEY_NUM1 = 321
KEY_NUM2 = 322
KEY_NUM3 = 323
KEY_NUM4 = 324
KEY_NUM5 = 325
KEY_NUM6 = 326
KEY_NUM7 = 327
KEY_NUM8 = 328
KEY_NUM9 = 329

'''
class Transform:
	def __init__(self, transform: glex.Transform):
		self.obj = transform	
	@property
	def position(self) -> glm.vec3:
		return glm.vec3(self.obj.position)	
	@position.setter
	def position(self, position: glm.vec3) -> None:
		self.obj.position = position.to_tuple()	
	@property
	def rotation(self) -> glm.quat:
		return glm.quat(self.obj.rotation)	
	@rotation.setter
	def rotation(self, rotation: glm.quat) -> None:
		self.obj.rotation = rotation.to_tuple()	
	@property
	def scale(self) -> glm.vec3:
		return glm.vec3(self.obj.scale)	
	@scale.setter
	def scale(self, scale: glm.vec3) -> None:
		self.obj.scale = scale.to_tuple()	
	def move(self, value: glm.vec3) -> None:
		self.obj.move(*value.to_tuple())
	@property
	def forward(self) -> glm.vec3:
		return self.rotation * glm.vec3(0.0, 0.0, 1.0)	
	@property
	def left(self) -> glm.vec3:
		return self.rotation * glm.vec3(1.0, 0.0, 0.0)

class GameObject:
	def __init__(self, obj: glex.GameObject):
		self.obj = obj
	@property
	def scene(self) -> glex.Scene:
		return self.obj.scene
	@property
	def transform(self) -> Transform:
		return Transform(self.obj.transform)
	
def script_method(fn):
	def actual_start(obj: glex.GameObject):
		fn(GameObject(obj))
	return actual_start

def get_rotation(begin: glm.vec3, end: glm.vec3) -> glm.quat:
	norm_u_norm_v = glm.sqrt(glm.dot(begin, begin) * glm.dot(end, end))
	real_part = norm_u_norm_v + glm.dot(begin, end)
	if real_part < 1e-6 * norm_u_norm_v:
		real_part = 0.0
		t = glm.vec3(-begin.y, begin.x, 0.0) if glm.abs(begin.x) > abs(begin.z) else glm.vec3(0.0, -begin.z, begin.y)
	else:
		t = glm.cross(begin, end)
	return glm.normalize(glm.quat(real_part, t.x, t.y, t.z))
'''