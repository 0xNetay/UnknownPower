
import os

OUTPUT = "./data/"
SYNC = OUTPUT + "sync"
LAST = OUTPUT + "last"
LOG  = OUTPUT + "log"
TICK = OUTPUT + "tick"


if not os.path.exists(OUTPUT):
    os.makedirs(OUTPUT)
