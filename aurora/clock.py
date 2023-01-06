import time


class Clock:
    def __init__(self, frequency: int|None = None) -> None:
        self.frequency = frequency

        self.start_time = self.get_time()
        self.time = self.get_time()

        self.fps = -1

    def start_tick(self) -> None:
        self.start_time = self.get_time()

    def tick(self) -> None:
        if self.frequency is not None:
            elapsed = self.get_time() - self.start_time
            waiting_time = (1_000_000_000 / self.frequency) - elapsed
            if waiting_time > 0:
                self.__sleep(waiting_time)

    def time_step(self) -> float:
        time = self.get_time()
        time_step = time - self.time
        self.fps = 1_000_000_000 / time_step
        
        self.time = time

        return time_step

    @staticmethod
    def __sleep(duration: float) -> None:
        now = time.perf_counter_ns()
        end = now + duration
        while now < end:
            now = time.perf_counter_ns()

    @staticmethod
    def get_time() -> float:
        return time.time_ns()

    def get_fps(self) -> float:
        return self.fps