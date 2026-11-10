import sys
from pathlib import Path

# Let the tests import asm4380 no matter where pytest is started from
sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
