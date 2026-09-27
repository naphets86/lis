# LIS∞ Scheduling Framework - Makefile
# 
# Kompiliert alle Module des LIS∞-Frameworks
# Basierend auf: Optimale Programmausführung mit lokaler Information
# Autor: Stephan Epp, 17. September 2026

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99 -g -pedantic
LDFLAGS = -lm

# Zielarchiv
BIN_DIR = ./bin
SRC_DIR = .

# Targets
TARGETS = $(BIN_DIR)/lis_scheduler $(BIN_DIR)/lis_extended $(BIN_DIR)/lis_verification

# Main target
all: $(BIN_DIR) $(TARGETS)

# Create bin directory
$(BIN_DIR):
	@mkdir -p $(BIN_DIR)
	@echo "[OK] Verzeichnis $(BIN_DIR) erstellt"

# Kompiliere Hauptprogramm (LIS∞ Scheduler)
$(BIN_DIR)/lis_scheduler: LISInfinityScheduler.c
	@echo "[COMPILE] LIS∞ Scheduling Framework..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "[OK] $(BIN_DIR)/lis_scheduler erstellt\n"

# Kompiliere erweiterte Module (Cache, Model Checking, Subgraph)
$(BIN_DIR)/lis_extended: LISIfinity.c
	@echo "[COMPILE] LIS∞ Extended (Cache, Model Checking)..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "[OK] $(BIN_DIR)/lis_extended erstellt\n"

# Kompiliere Verifikation (State Space, Deadlock, Benchmarking)
$(BIN_DIR)/lis_verification: LISInfinityVerification.c
	@echo "[COMPILE] LIS∞ Verifikation und Benchmarking..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "[OK] $(BIN_DIR)/lis_verification erstellt\n"

# Run all programs
run: all
	@echo "=============================================="
	@echo "LIS∞ SCHEDULER - AUSFÜHRUNG"
	@echo "=============================================="
	@echo ""
	$(BIN_DIR)/lis_scheduler
	@echo ""
	@echo "=============================================="
	@echo "LIS∞ EXTENDED (Cache & Model Checking)"
	@echo "=============================================="
	@echo ""
	$(BIN_DIR)/lis_extended
	@echo ""
	@echo "=============================================="
	@echo "LIS∞ VERIFICATION & BENCHMARKING"
	@echo "=============================================="
	@echo ""
	$(BIN_DIR)/lis_verification

# Run einzelne Programme
run_scheduler: $(BIN_DIR)/lis_scheduler
	$(BIN_DIR)/lis_scheduler

run_extended: $(BIN_DIR)/lis_extended
	$(BIN_DIR)/lis_extended

run_verification: $(BIN_DIR)/lis_verification
	$(BIN_DIR)/lis_verification

# Bereinigung
clean:
	@echo "[INFO] Räume auf..."
	@rm -rf $(BIN_DIR)
	@echo "[OK] Bereinigung abgeschlossen"

# Hilfe
help:
	@echo "=============================================="
	@echo "LIS∞ MAKEFILE HILFE"
	@echo "=============================================="
	@echo ""
	@echo "Verfügbare Ziele:"
	@echo "  all              - Kompiliere alle Module"
	@echo "  run              - Führe alle Programme aus"
	@echo "  run_scheduler    - Führe LIS∞ Scheduler aus"
	@echo "  run_extended     - Führe LIS∞ Extended aus"
	@echo "  run_verification - Führe LIS∞ Verification aus"
	@echo "  clean            - Lösche kompilierte Binaries"
	@echo "  help             - Zeige diese Hilfe"
	@echo ""
	@echo "Beispiele:"
	@echo "  make             # Kompiliert alles"
	@echo "  make run         # Führt alle Tests aus"
	@echo "  make clean       # Räumt auf"

# Phony targets
.PHONY: all run run_scheduler run_extended run_verification clean help
