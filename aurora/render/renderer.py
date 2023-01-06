from pygame.surface import Surface
from pygame import SRCALPHA
from pygame.transform import scale

from .camera import Camera
from ..entity import RenderedEntity

from gravity.math.vector import Vector2


class Renderer:
    def __init__(self, screen_size: Vector2, entities: list[RenderedEntity]) -> None:
        self.screen_size = screen_size
        self.entities = entities
        self.camera = Camera(Vector2(), screen_size)
    
    def render(self) -> Surface:
        surface = Surface(tuple(self.screen_size), SRCALPHA)

        for entity in self.entities:
            surface.blit(self.resize(entity.texture.get_surface(), 1), tuple(self.camera.convert_position(entity.position - Vector2(entity.texture.surface.get_width() / 2, entity.texture.surface.get_height() / 2))))

        return surface
    
    def resize(self, surface: Surface, ratio: float) -> Surface:
        return scale(surface, (int(surface.get_width() * ratio), int(surface.get_height() * ratio)))