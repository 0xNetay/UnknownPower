
class SifterException(Exception):
    pass


class InjectorClosed(SifterException):
    pass


class InjectorDoesntExist(SifterException):
    @classmethod
    def from_path(cls, path):
        return cls(f"Injector binary wasn't found at {path}")
