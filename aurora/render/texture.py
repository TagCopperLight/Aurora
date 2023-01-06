from pygame.surface import Surface
from pygame.image import load
from pygame.transform import scale


class Texture:
    def __init__(self, file_path: str) -> None:
        self.file_path = file_path
        self.surface = Surface((0, 0))

    def get_texture(self) -> Surface:
        raw_image = load(self.file_path).convert_alpha()

        return scale(raw_image.copy(), (30, 30))

    def get_surface(self) -> Surface:
        return self.surface