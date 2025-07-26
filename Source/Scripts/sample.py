import gscript as gs
import random
import math

transform = None

def start(self: gs.GameObject):
    # Aya with a point light attached to the camera.
    gs.set_title("选择你喜欢的颜色！")
    global transform
    transform = self.transform

def update(self: gs.GameObject):
    if (gs.pressed(gs.KEY_RETURN)):
        self.scene.set_clearcolor(random.random(), random.random(), random.random())
    scale = math.sin(gs.time() * 0.001) * 0.1 + 0.5
    transform.scale = (scale, scale, scale)