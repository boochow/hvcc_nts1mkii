from hvccloguesdk import LogueSDKV2Generator

class Nts1mkii_del(LogueSDKV2Generator):
    FIXED_PARAMS = ("shape", "alt")
    BUILTIN_PARAMS = ("pitch", "pitch_note", "noteon_trig", "noteoff_trig", "slfo")
    UNIT_NUM_OUTPUT = 1
    MAX_SDRAM_SIZE = 0
    SDRAM_ALLOC_THRESHOLD = 0
    MAX_UNIT_SIZE = 49152

    def unit_type():
        return "osc"
