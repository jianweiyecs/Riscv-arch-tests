# NanHuV5 is a concrete verification target, not a framework concept.
# Keep its defaults here so generic builds and other CPU targets stay decoupled.

PLAT ?= linknan
TEST_MARCH ?= rv64imac_zicsr_zifencei_zicbom_zicboz_zve64x
TEST_MABI ?= lp64
PAGE_TABLE_MODE ?= sv48
PAGE_TABLE_BACKEND ?= static
