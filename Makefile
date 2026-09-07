PYTHON ?= python3
COMPILER_DIR ?= /opt/pawn-3.2.3664
SDK_INCLUDE ?= /opt/samp-sdk/include

.PHONY: build database syntax checksums

build:
	./scripts/build.sh --compiler-dir "$(COMPILER_DIR)" --sdk-include "$(SDK_INCLUDE)"

database:
	$(PYTHON) scripts/create-clean-db.py --schema database/schema.sql --output database/accounts.clean.db --force
	install -D -m 0640 database/accounts.clean.db server-files/scriptfiles/accounts.db

syntax:
	bash -n scripts/build.sh scripts/install.sh scripts/verify.sh scripts/install-dependencies-debian.sh
	$(PYTHON) -m py_compile scripts/create-clean-db.py scripts/samp_query.py

checksums:
	$(PYTHON) scripts/generate-checksums.py
