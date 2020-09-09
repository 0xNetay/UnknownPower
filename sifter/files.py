
import os
import pathlib
from functools import lru_cache

_OUTPUT = "./data/"
_SYNC = _OUTPUT + "sync"
_LAST = _OUTPUT + "last"
_LOG  = _OUTPUT + "log"
_INJECTOR = "../injector/UnknownPower"


@lru_cache()
def get_sync_file_path():
    return _get_file_path(_SYNC)


@lru_cache()
def get_last_file_path():
    return _get_file_path(_LAST)


@lru_cache()
def get_log_file_path():
    return _get_file_path(_LOG)


@lru_cache()
def get_injector_file_path():
    return _get_file_path(_INJECTOR)


def _get_file_path(file_relative_path):
    return pathlib.Path(__file__).parent.joinpath(file_relative_path).absolute()


if not os.path.exists(_OUTPUT):
    os.makedirs(_OUTPUT)
