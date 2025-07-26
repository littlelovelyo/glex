import gscript as gs
import glm

camtrans = None
speed_multiplier = 10
move_speed = 0.0005
mouse_sensitivity = 0.001
start_position = glm.vec3(0.0, 0.5, 3.0)
look_at = glm.vec3(0.0, 0.0, 0.0)
rotation = None
inside = False
vsync = True

@gs.script_method
def start(camera: gs.GameObject):
	global camtrans, rotation
	camtrans = camera.transform
	camtrans.position = start_position
	rot = gs.get_rotation(glm.vec3(0.0, 0.0, 1.0), look_at - start_position)
	camtrans.rotation = rot
	rotation = glm.eulerAngles(rot)

@gs.script_method
def update(camera: gs.GameObject):
	global speed_multiplier
	speed_multiplier -= gs.mouse_scroll() * 10;
	speed = move_speed * speed_multiplier

	if gs.pressing(gs.KEY_W):
		camtrans.move(camtrans.forward * gs.delta_time() * speed)
	elif gs.pressing(gs.KEY_S):
		camtrans.move(camtrans.forward * gs.delta_time() * -speed)
	if gs.pressing(gs.KEY_A):
		camtrans.move(camtrans.left * gs.delta_time() * -speed)
	elif gs.pressing(gs.KEY_D):
		camtrans.move(camtrans.left * gs.delta_time() * speed)
	if gs.pressing(gs.KEY_SPACE):
		camtrans.move(glm.vec3(0.0, gs.delta_time() * speed, 0.0))
	elif gs.pressing(gs.KEY_LEFT_SHIFT):
		camtrans.move(glm.vec3(0.0, gs.delta_time() * -speed, 0.0))
	
	global rotation, inside
	if gs.pressed(gs.KEY_C):
		gs.capture_mouse()
		inside = True
	elif gs.pressed(gs.KEY_R):
		gs.free_mouse()
		inside = False
	if inside:
		rotation.x -= gs.mouse_deltay() * mouse_sensitivity
		rotation.y -= gs.mouse_deltax() * mouse_sensitivity
		camtrans.rotation = glm.quat(rotation)

	global vsync
	if gs.pressed(gs.KEY_RETURN):
		vsync = not vsync
		gs.set_vsync(vsync)