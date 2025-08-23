from hvccloguesdk import LogueSDKV2Generator

class Nts1mkii_del(LogueSDKV2Generator):
    FIXED_PARAMS = ("time", "depth")
    UNIT_NUM_OUTPUT = 2
    MAX_SDRAM_SIZE = 262144
    MAX_UNIT_SIZE = 16384

    def unit_type():
        return "modfx"
