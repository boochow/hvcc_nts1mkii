from hvccloguesdk import LogueSDKV2Generator

class Nts1mkii_del(LogueSDKV2Generator):
    FIXED_PARAMS = ("time", "depth", "mix")
    UNIT_NUM_OUTPUT = 2
    MAX_SDRAM_SIZE = 3145728
    MAX_UNIT_SIZE = 24576
    MSG_POOL_ON_SRAM = True

    def unit_type():
        return "revfx"
