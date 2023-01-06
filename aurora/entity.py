from .render.texture import Texture

from gravity.math.vector import Vector2


class RenderedEntity:
    def __init__(self, position: Vector2, texture: Texture) -> None:
        self.position = position
        self.texture = texture