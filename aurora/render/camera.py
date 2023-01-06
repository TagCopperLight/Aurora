from gravity.math.vector import Vector2


class Camera:
    def __init__(self, position: Vector2, screen_size: Vector2) -> None:
        self.position = position
        self.screen_size = screen_size
        self.zoom = 1

    def convert_position(self, position: Vector2) -> Vector2:
        return (position - self.position) * self.zoom + self.screen_size / 2