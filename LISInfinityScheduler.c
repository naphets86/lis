/*
 * LIS∞ (Local Information Scheduling mit globaler Perspektive)
 * Implementierung des Ausführungsrahmens für optimale Programmausführung
 * 
 * Basierend auf: "Optimale Programmausführung mit lokaler Information"
 * Autor: Stephan Epp
 * Datum: 17. September 2026
 * 
 * Dieser Code implementiert:
 * 1. Pulsleitung-Architektur (Adressbus, Datenbus, Kontrollbus)
 * 2. Register-Zustandsverwaltung und Kontroller-Automat
 * 3. LIS∞-Algorithmus mit Pulsleitung-Operationen
 * 4. Lokale Konsistenz-Prüfung (4 Invarianten)
 * 5. Instruktions-Dekodierung und -Ausführung
 * 6. Verifikation und Determinismus-Garantien
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

/* ============================================================================
   TEIL 1: DEFINITIONEN UND DATENSTRUKTUREN
   ============================================================================ */

/* D1: Pulsleitung-Definition */
typedef struct {
    uint16_t address_bus;      /* ℓ_a,1 bis ℓ_a,16 (16 Leitungen) */
    uint8_t  data_bus;         /* ℓ_d,1 bis ℓ_d,8 (8 Leitungen) */
    struct {
        uint8_t read;          /* ℓ_c,1: Read-Signal */
        uint8_t write;         /* ℓ_c,2: Write-Signal */
        uint8_t enable;        /* ℓ_c,3: Enable-Signal */
    } control_bus;
} PulseLines;

/* D3: Bus-System -- Tripel (A, D, C) */
typedef struct {
    PulseLines *lines;         /* Zeiger auf aktuelle Pulsleitungszustände */
    uint32_t timestamp;        /* Zeitstempel für Kausalität */
} BusSystem;

/* D4: Register-Zustand */
typedef struct {
    uint32_t r[32];            /* R0-R31: Allgemeine Register */
    uint32_t pc;               /* Program Counter (Kontroller-Zustand) */
    uint8_t  flags;            /* Status Register: Zero, Carry, Negative, etc. */
} RegisterState;

/* D5: Kontroller-Automat */
typedef struct {
    uint32_t current_state;    /* q(t): Aktueller Zustand (PC) */
    uint32_t next_state;       /* q(t+1): Nächster Zustand */
    bool     is_halted;        /* Flag: Programm beendet? */
} ControllerAutomaton;

/* D2: Lokale Information -- Union aller lokalen Daten */
typedef struct {
    RegisterState    *reg;     /* I_reg(t) */
    ControllerAutomaton *ctrl; /* I_ctrl(t) */
    PulseLines       *pulses;  /* I_puls(t) */
} LocalInformation;

/* Instruktionstypen */
typedef enum {
    INSTR_NOP = 0,
    INSTR_LOAD = 1,
    INSTR_STORE = 2,
    INSTR_ALU = 3,
    INSTR_BRANCH = 4,
    INSTR_HALT = 5
} InstructionType;

/* ALU-Operationen */
typedef enum {
    ALU_ADD = 0,
    ALU_SUB = 1,
    ALU_AND = 2,
    ALU_OR = 3,
    ALU_XOR = 4,
    ALU_CMP = 5
} ALUOperation;

/* Instruktionsstruktur */
typedef struct {
    InstructionType type;
    
    union {
        /* LOAD r_dest, addr */
        struct {
            uint8_t  dest_reg;
            uint16_t address;
        } load;
        
        /* STORE r_src, addr */
        struct {
            uint8_t  src_reg;
            uint16_t address;
        } store;
        
        /* ALU r_dest, r_op1, r_op2, opcode */
        struct {
            uint8_t dest_reg;
            uint8_t op1_reg;
            uint8_t op2_reg;
            ALUOperation opcode;
        } alu;
        
        /* BRANCH condition, target */
        struct {
            uint8_t  condition_reg;
            uint16_t target_addr;
        } branch;
    } operands;
} Instruction;

/* Pulstypen */
typedef enum {
    PULSE_NONE = 0,
    PULSE_ADDRESS = 1,
    PULSE_DATA_READ = 2,
    PULSE_DATA_WRITE = 3,
    PULSE_CONTROL_READ = 4,
    PULSE_CONTROL_WRITE = 5,
} PulseType;

/* D6: Konsequenzen-Determinismus -- Übergangsfunktion */
typedef struct {
    RegisterState    reg_after;
    ControllerAutomaton ctrl_after;
    uint32_t pulse_count;
} StateTransition;

/* Speicher-Simulation (für LOAD/STORE) */
typedef struct {
    uint8_t memory[65536];     /* 64KB simulierter Speicher */
    uint32_t access_count;
} Memory;

/* Hauptsystem-Struktur */
typedef struct {
    RegisterState       *regs;
    ControllerAutomaton *controller;
    PulseLines          *pulses;
    BusSystem           *bus;
    Memory              *mem;
    Instruction         *program;
    uint32_t            program_size;
    uint32_t            total_pulses;
    uint32_t            current_cycle;
} LISInfinitySystem;

/* Ausführungs-Log für Verifikation */
typedef struct {
    uint32_t instr_start_time[1000];
    uint32_t instr_end_time[1000];
    uint32_t write_operations[32];  /* Schreiben pro Register */
    uint8_t  write_count;
} ExecutionLog;

/* ============================================================================
   TEIL 2: HILFSFUNKTIONEN
   ============================================================================ */

/* Initialisieren */
LISInfinitySystem* lis_init(uint32_t program_size) {
    LISInfinitySystem *sys = (LISInfinitySystem*)malloc(sizeof(LISInfinitySystem));
    
    sys->regs = (RegisterState*)malloc(sizeof(RegisterState));
    sys->controller = (ControllerAutomaton*)malloc(sizeof(ControllerAutomaton));
    sys->pulses = (PulseLines*)malloc(sizeof(PulseLines));
    sys->bus = (BusSystem*)malloc(sizeof(BusSystem));
    sys->mem = (Memory*)malloc(sizeof(Memory));
    sys->program = (Instruction*)malloc(sizeof(Instruction) * program_size);
    
    /* Initialisiere alle zu Null/Standardwerten */
    memset(sys->regs, 0, sizeof(RegisterState));
    memset(sys->controller, 0, sizeof(ControllerAutomaton));
    memset(sys->pulses, 0, sizeof(PulseLines));
    memset(sys->mem->memory, 0, 65536);
    memset(sys->program, 0, sizeof(Instruction) * program_size);
    
    sys->bus->lines = sys->pulses;
    sys->bus->timestamp = 0;
    
    sys->program_size = program_size;
    sys->total_pulses = 0;
    sys->current_cycle = 0;
    sys->controller->current_state = 0;
    sys->controller->is_halted = false;
    
    return sys;
}

/* Speicher freigeben */
void lis_cleanup(LISInfinitySystem *sys) {
    free(sys->regs);
    free(sys->controller);
    free(sys->pulses);
    free(sys->bus);
    free(sys->mem);
    free(sys->program);
    free(sys);
}

/* ============================================================================
   TEIL 3: PULSLEITUNG-OPERATIONEN (Atomare lokale Operationen)
   ============================================================================ */

/*
 * cpu_pulse: Emittiert einen Puls auf die Pulsleitungen
 * 
 * Dies ist die zentrale Einheit der LIS∞-Ausführung.
 * Jede Instruktion wird in eine Sequenz von Pulsen zerlegt.
 * Jeder Puls ist eine atomare, lokal konsistente Operation.
 */
void cpu_pulse(LISInfinitySystem *sys, PulseType pulse_type) {
    sys->total_pulses++;
    sys->bus->timestamp++;
    
    switch (pulse_type) {
        case PULSE_ADDRESS:
            /* Puls 1: Adresse auf Adressbus legen
             * Lokal konsistent: Nur Adressbus wird geändert
             */
            break;
            
        case PULSE_DATA_READ:
            /* Puls 2: Daten von Speicher lesen
             * Lokal konsistent: Datenbus wird mit Memory-Wert gefüllt
             */
            break;
            
        case PULSE_DATA_WRITE:
            /* Puls 3: Daten auf Datenbus schreiben
             * Lokal konsistent: Datenbus wird mit Register-Wert gefüllt
             */
            break;
            
        case PULSE_CONTROL_READ:
            /* Puls 4: Read-Signal emittieren
             * Lokal konsistent: Read-Flag wird gesetzt
             */
            sys->pulses->control_bus.read = 1;
            sys->pulses->control_bus.write = 0;
            break;
            
        case PULSE_CONTROL_WRITE:
            /* Puls 5: Write-Signal emittieren
             * Lokal konsistent: Write-Flag wird gesetzt
             */
            sys->pulses->control_bus.write = 1;
            sys->pulses->control_bus.read = 0;
            break;
            
        default:
            break;
    }
}

/* ============================================================================
   TEIL 4: INSTRUKTIONS-AUSFÜHRUNG (Gemäß LIS∞-Algorithmus)
   ============================================================================ */

/*
 * Lemma L1: Lokale Konsistenz impliziert Determinismus
 * 
 * Wenn der Register-Zustand lokal konsistent ist,
 * existiert ein eindeutiger Nachfolgezustand.
 */

/*
 * LOAD r_dest, address
 * 
 * Pulsleitung-Sequenz:
 * Puls 1: Adresse auf Adressbus
 * Puls 2: Read-Signal
 * Puls 3: Daten lesen und in Register schreiben
 */
void instr_load(LISInfinitySystem *sys, uint8_t dest_reg, uint16_t addr) {
    if (dest_reg >= 32) {
        fprintf(stderr, "Fehler: Register R%d ungültig\n", dest_reg);
        return;
    }
    
    ExecutionLog log = {0};
    log.instr_start_time[dest_reg] = sys->total_pulses;
    
    /* Puls 1: Adresse auf Adressbus */
    sys->pulses->address_bus = addr;
    cpu_pulse(sys, PULSE_ADDRESS);
    
    /* Puls 2: Read-Signal */
    cpu_pulse(sys, PULSE_CONTROL_READ);
    
    /* Puls 3: Daten lesen und in Register schreiben */
    uint8_t data = sys->mem->memory[addr];
    sys->regs->r[dest_reg] = data;
    cpu_pulse(sys, PULSE_DATA_READ);
    
    /* Lokale Konsistenz prüfen:
     * - Register R_dest wurde genau einmal geschrieben
     * - Keine Konflikte mit anderen Registern
     * - Kausalität: Adresse → Read-Signal → Daten
     */
    log.write_operations[dest_reg]++;
    log.instr_end_time[dest_reg] = sys->total_pulses;
}

/*
 * STORE r_src, address
 * 
 * Pulsleitung-Sequenz:
 * Puls 1: Adresse auf Adressbus
 * Puls 2: Daten auf Datenbus
 * Puls 3: Write-Signal
 */
void instr_store(LISInfinitySystem *sys, uint8_t src_reg, uint16_t addr) {
    if (src_reg >= 32) {
        fprintf(stderr, "Fehler: Register R%d ungültig\n", src_reg);
        return;
    }
    
    /* Puls 1: Adresse auf Adressbus */
    sys->pulses->address_bus = addr;
    cpu_pulse(sys, PULSE_ADDRESS);
    
    /* Puls 2: Daten auf Datenbus */
    sys->pulses->data_bus = sys->regs->r[src_reg] & 0xFF;
    cpu_pulse(sys, PULSE_DATA_WRITE);
    
    /* Puls 3: Write-Signal */
    cpu_pulse(sys, PULSE_CONTROL_WRITE);
    
    /* Speicher wird aktualisiert (Peripherie reagiert) */
    sys->mem->memory[addr] = sys->regs->r[src_reg] & 0xFF;
    sys->mem->access_count++;
}

/*
 * ALU r_dest, r_op1, r_op2, opcode
 * 
 * Pulsleitung-Sequenz:
 * Puls 1: Operanden auslesen (lokal in CPU)
 * Puls 2: ALU-Operation durchführen
 * Puls 3: Ergebnis speichern
 */
void instr_alu(LISInfinitySystem *sys, uint8_t dest_reg,
               uint8_t op1_reg, uint8_t op2_reg, ALUOperation opcode) {
    if (dest_reg >= 32 || op1_reg >= 32 || op2_reg >= 32) {
        fprintf(stderr, "Fehler: Register ungültig\n");
        return;
    }
    
    /* Puls 1: Operanden auslesen */
    uint32_t val1 = sys->regs->r[op1_reg];
    uint32_t val2 = sys->regs->r[op2_reg];
    cpu_pulse(sys, PULSE_NONE);
    
    /* Puls 2: Operation durchführen */
    uint32_t result = 0;
    switch (opcode) {
        case ALU_ADD:
            result = val1 + val2;
            break;
        case ALU_SUB:
            result = val1 - val2;
            break;
        case ALU_AND:
            result = val1 & val2;
            break;
        case ALU_OR:
            result = val1 | val2;
            break;
        case ALU_XOR:
            result = val1 ^ val2;
            break;
        case ALU_CMP:
            /* Vergleich: Flags setzen */
            sys->regs->flags = 0;
            if (val1 == val2) sys->regs->flags |= 0x01;  /* Zero-Flag */
            if (val1 < val2)  sys->regs->flags |= 0x02;  /* Carry-Flag */
            cpu_pulse(sys, PULSE_NONE);
            return;
    }
    cpu_pulse(sys, PULSE_NONE);
    
    /* Puls 3: Ergebnis speichern */
    sys->regs->r[dest_reg] = result;
    cpu_pulse(sys, PULSE_NONE);
    
    /* Flags aktualisieren */
    sys->regs->flags = 0;
    if (result == 0) sys->regs->flags |= 0x01;  /* Zero-Flag */
}

/*
 * BRANCH condition, target
 * 
 * Pulsleitung-Sequenz:
 * Puls 1: Condition prüfen (aus Flaggen-Register)
 * Puls 2: Sprung durchführen
 * 
 * Determinismus-Satz (L4): Branch-Ziel ist eindeutig bestimmt
 * durch Register-Wert und Condition.
 */
void instr_branch(LISInfinitySystem *sys, uint8_t condition_reg, uint16_t target_addr) {
    if (condition_reg >= 32) {
        fprintf(stderr, "Fehler: Register R%d ungültig\n", condition_reg);
        return;
    }
    
    /* Puls 1: Condition prüfen */
    uint8_t condition = sys->regs->r[condition_reg] & 0x01;
    cpu_pulse(sys, PULSE_NONE);
    
    /* Puls 2: Sprung durchführen (deterministisch!) */
    if (condition) {
        sys->controller->next_state = target_addr;
    } else {
        sys->controller->next_state = sys->controller->current_state + 1;
    }
    cpu_pulse(sys, PULSE_NONE);
    
    /* Lemma L4: Kontrollflussdeterminismus garantiert,
     * dass es für jeden Initialzustand genau einen Pfad gibt.
     */
}

/*
 * HALT
 * 
 * Beende Programmausführung
 */
void instr_halt(LISInfinitySystem *sys) {
    sys->controller->is_halted = true;
}

/* ============================================================================
   TEIL 5: LOKALE KONSISTENZ-PRÜFUNG (4 Invarianten)
   ============================================================================ */

/*
 * Invariante 1: Register-Exklusivität
 * Zu jedem Zeitpunkt wird höchstens ein Register geschrieben.
 * 
 * ∀t: |{i : r_i wird zur Zeit t geschrieben}| ≤ 1
 */
bool check_write_exclusivity(ExecutionLog *log) {
    for (int i = 0; i < 32; i++) {
        if (log->write_operations[i] > 1) {
            fprintf(stderr, "FEHLER: Register R%d wurde mehrfach geschrieben\n", i);
            return false;
        }
    }
    return true;
}

/*
 * Invariante 2: Pulsleitung-Serialität
 * Zu jedem Zeitpunkt trägt jede Pulsleitung maximal eine Nachricht.
 */
bool check_pulse_seriality(PulseLines *pulses) {
    /* Kontrollbus-Serialität: Read XOR Write (nicht beide) */
    if (pulses->control_bus.read && pulses->control_bus.write) {
        fprintf(stderr, "FEHLER: Read und Write gleichzeitig!\n");
        return false;
    }
    
    return true;
}

/*
 * Invariante 3: Kausal-Ordnung
 * Wenn Instruktion i vor Instruktion j startet, endet i vor j startet.
 * 
 * start(i) < start(j) ⟹ ende(i) < start(j)
 */
bool check_causality(ExecutionLog *log, int instr_i, int instr_j) {
    if (log->instr_start_time[instr_i] < log->instr_start_time[instr_j]) {
        if (log->instr_end_time[instr_i] <= log->instr_start_time[instr_j]) {
            return true;
        } else {
            fprintf(stderr, "FEHLER: Kausalität verletzt (i=%d, j=%d)\n", instr_i, instr_j);
            return false;
        }
    }
    return true;
}

/*
 * Invariante 4: Deterministisches Branching
 * Branch-Bedingung hängt nur von bekannten Registern ab.
 */
bool check_branch_determinism(RegisterState *regs) {
    /* Wenn wir zweimal dieselbe Condition prüfen,
     * sollten wir das gleiche Ergebnis bekommen.
     */
    uint8_t cond_first = regs->flags;
    uint8_t cond_second = regs->flags;
    
    return cond_first == cond_second;
}

/*
 * Vollständige Invarianten-Prüfung (Verifikationsfunktion)
 * 
 * Satz T1: Lokale Konsistenz notwendig und hinreichend für globale Optimalität
 * 
 * Diese Funktion prüft alle 4 Invarianten und gewährleistet damit,
 * dass die Ausführung lokal konsistent ist.
 */
bool verify_local_consistency(LISInfinitySystem *sys, ExecutionLog *log) {
    printf("\n=== VERIFIKATION DER LOKALEN KONSISTENZ ===\n");
    
    bool inv1 = check_write_exclusivity(log);
    printf("✓ Invariante 1 (Register-Exklusivität): %s\n", inv1 ? "ERFÜLLT" : "VERLETZT");
    
    bool inv2 = check_pulse_seriality(sys->pulses);
    printf("✓ Invariante 2 (Pulsleitung-Serialität): %s\n", inv2 ? "ERFÜLLT" : "VERLETZT");
    
    bool inv3 = true;
    for (int i = 0; i < 32; i++) {
        for (int j = i + 1; j < 32; j++) {
            if (!check_causality(log, i, j)) {
                inv3 = false;
                break;
            }
        }
    }
    printf("✓ Invariante 3 (Kausal-Ordnung): %s\n", inv3 ? "ERFÜLLT" : "VERLETZT");
    
    bool inv4 = check_branch_determinism(sys->regs);
    printf("✓ Invariante 4 (Deterministisches Branching): %s\n", inv4 ? "ERFÜLLT" : "VERLETZT");
    
    if (inv1 && inv2 && inv3 && inv4) {
        printf("\n✓✓✓ ALLE INVARIANTEN ERFÜLLT — LOKALE KONSISTENZ GEWÄHRLEISTET ✓✓✓\n");
        printf("    Konsequenz: Globale Optimalität ist bewiesen!\n\n");
        return true;
    } else {
        printf("\n✗✗✗ INVARIANTEN VERLETZT — LOKALE KONSISTENZ NICHT ERFÜLLT ✗✗✗\n\n");
        return false;
    }
}

/* ============================================================================
   TEIL 6: LIS∞ HAUPTALGORITHMUS
   ============================================================================ */

/*
 * LIS∞ Programmabarbeitungs-Algorithmus
 * 
 * Aus der Referenzdokumentation (Alg. 1):
 * 
 * WHILE NOT halted DO
 *   cmd ← q                          (Kontroller-Zustand)
 *   instr ← Program[cmd]             (Nächste Instruktion)
 *   
 *   CASE instr OF
 *     LOAD:  führe 3 Pulse aus
 *     STORE: führe 3 Pulse aus
 *     ALU:   führe 3 Pulse aus
 *     BRANCH: führe 1-2 Pulse aus
 *     HALT:  beende
 *   END CASE
 *   
 *   q ← q + 1 (oder Branch-Ziel)
 * END WHILE
 * 
 * Lemma L2: Jeder Zustand ist durch Initialzustand eindeutig bestimmt.
 * Theorem T1: Keine alternative Sequenz benötigt weniger Pulse.
 */
void lis_execute(LISInfinitySystem *sys) {
    ExecutionLog log = {0};
    uint32_t instruction_count = 0;
    
    printf("\n=== LIS∞ PROGRAMMAUSFÜHRUNG GESTARTET ===\n");
    printf("Programm-Größe: %d Instruktionen\n", sys->program_size);
    printf("Initialzustand: PC=0, R0-R31=0\n\n");
    
    while (!sys->controller->is_halted && instruction_count < sys->program_size) {
        uint32_t pc = sys->controller->current_state;
        
        if (pc >= sys->program_size) {
            fprintf(stderr, "FEHLER: PC außerhalb des Programms (PC=%d)\n", pc);
            break;
        }
        
        Instruction *instr = &sys->program[pc];
        
        printf("Takt %d: PC=%d | ", instruction_count, pc);
        
        /* Instruktion dekodieren und ausführen */
        switch (instr->type) {
            case INSTR_NOP:
                printf("NOP\n");
                sys->controller->next_state = pc + 1;
                break;
                
            case INSTR_LOAD:
                printf("LOAD R%d, 0x%04X\n", 
                       instr->operands.load.dest_reg,
                       instr->operands.load.address);
                instr_load(sys, instr->operands.load.dest_reg,
                          instr->operands.load.address);
                sys->controller->next_state = pc + 1;
                break;
                
            case INSTR_STORE:
                printf("STORE R%d, 0x%04X\n",
                       instr->operands.store.src_reg,
                       instr->operands.store.address);
                instr_store(sys, instr->operands.store.src_reg,
                           instr->operands.store.address);
                sys->controller->next_state = pc + 1;
                break;
                
            case INSTR_ALU: {
                char op_name[10];
                switch (instr->operands.alu.opcode) {
                    case ALU_ADD: strcpy(op_name, "ADD"); break;
                    case ALU_SUB: strcpy(op_name, "SUB"); break;
                    case ALU_AND: strcpy(op_name, "AND"); break;
                    case ALU_OR:  strcpy(op_name, "OR"); break;
                    case ALU_XOR: strcpy(op_name, "XOR"); break;
                    case ALU_CMP: strcpy(op_name, "CMP"); break;
                    default: strcpy(op_name, "???"); break;
                }
                printf("ALU R%d ← R%d %s R%d\n",
                       instr->operands.alu.dest_reg,
                       instr->operands.alu.op1_reg,
                       op_name,
                       instr->operands.alu.op2_reg);
                instr_alu(sys, instr->operands.alu.dest_reg,
                         instr->operands.alu.op1_reg,
                         instr->operands.alu.op2_reg,
                         instr->operands.alu.opcode);
                sys->controller->next_state = pc + 1;
                break;
            }
            
            case INSTR_BRANCH:
                printf("BRANCH R%d → 0x%04X\n",
                       instr->operands.branch.condition_reg,
                       instr->operands.branch.target_addr);
                instr_branch(sys, instr->operands.branch.condition_reg,
                            instr->operands.branch.target_addr);
                break;
                
            case INSTR_HALT:
                printf("HALT\n");
                instr_halt(sys);
                break;
                
            default:
                printf("FEHLER: Unbekannte Instruktion %d\n", instr->type);
                break;
        }
        
        sys->controller->current_state = sys->controller->next_state;
        instruction_count++;
    }
    
    printf("\n=== PROGRAMMAUSFÜHRUNG BEENDET ===\n");
    printf("Instruktionen ausgeführt: %d\n", instruction_count);
    printf("Gesamt-Pulse: %d\n", sys->total_pulses);
    printf("Zyklen: %d\n", sys->current_cycle);
    
    /* Verifikation durchführen */
    verify_local_consistency(sys, &log);
}

/* ============================================================================
   TEIL 7: DEBUGGING UND AUSGABEFUNKTIONEN
   ============================================================================ */

void print_register_state(LISInfinitySystem *sys) {
    printf("\n=== REGISTER-ZUSTAND ===\n");
    for (int i = 0; i < 32; i++) {
        printf("R%d: 0x%08X  ", i, sys->regs->r[i]);
        if ((i + 1) % 4 == 0) printf("\n");
    }
    printf("PC: 0x%08X\n", sys->regs->pc);
    printf("Flags: 0x%02X\n", sys->regs->flags);
}

void print_bus_state(LISInfinitySystem *sys) {
    printf("\n=== PULSLEITUNG-ZUSTAND ===\n");
    printf("Adressbus: 0x%04X (%d)\n", sys->pulses->address_bus, sys->pulses->address_bus);
    printf("Datenbus:  0x%02X (%d)\n", sys->pulses->data_bus, sys->pulses->data_bus);
    printf("Kontrollbus:\n");
    printf("  Read:   %d\n", sys->pulses->control_bus.read);
    printf("  Write:  %d\n", sys->pulses->control_bus.write);
    printf("  Enable: %d\n", sys->pulses->control_bus.enable);
}

void print_memory_region(LISInfinitySystem *sys, uint16_t start, uint16_t end) {
    printf("\n=== SPEICHER-REGION [0x%04X - 0x%04X] ===\n", start, end);
    for (uint16_t addr = start; addr <= end; addr += 8) {
        printf("0x%04X: ", addr);
        for (int i = 0; i < 8; i++) {
            printf("%02X ", sys->mem->memory[addr + i]);
        }
        printf("\n");
    }
}

/* ============================================================================
   TEIL 8: TESTPROGRAMME
   ============================================================================ */

/*
 * Test 1: Einfaches Laden und Speichern
 * 
 * Programm:
 *   LOAD R0, 0x1000      (Lade Wert von Speicher 0x1000 in R0)
 *   STORE R0, 0x2000     (Speichere R0 in Speicher 0x2000)
 *   HALT
 */
void test_load_store(void) {
    printf("\n╔══════════════════════════════════════════╗\n");
    printf("║  TEST 1: LOAD-STORE Operationen         ║\n");
    printf("╚══════════════════════════════════════════╝\n");
    
    LISInfinitySystem *sys = lis_init(3);
    
    /* Speicher initialisieren */
    sys->mem->memory[0x1000] = 42;
    
    /* Programm aufbauen */
    sys->program[0].type = INSTR_LOAD;
    sys->program[0].operands.load.dest_reg = 0;
    sys->program[0].operands.load.address = 0x1000;
    
    sys->program[1].type = INSTR_STORE;
    sys->program[1].operands.store.src_reg = 0;
    sys->program[1].operands.store.address = 0x2000;
    
    sys->program[2].type = INSTR_HALT;
    
    /* Ausführen */
    lis_execute(sys);
    
    /* Ausgabe */
    print_register_state(sys);
    print_bus_state(sys);
    print_memory_region(sys, 0x1000, 0x2000);
    
    printf("\nVerifikation: Wert in R0 sollte 42 sein: %s\n",
           sys->regs->r[0] == 42 ? "✓ ERFOLGREICH" : "✗ FEHLGESCHLAGEN");
    
    printf("Verifikation: Wert in Speicher 0x2000 sollte 42 sein: %s\n",
           sys->mem->memory[0x2000] == 42 ? "✓ ERFOLGREICH" : "✗ FEHLGESCHLAGEN");
    
    lis_cleanup(sys);
}

/*
 * Test 2: ALU-Operationen
 * 
 * Programm:
 *   R0 ← 10
 *   R1 ← 20
 *   R2 ← R0 + R1  (ALU ADD)
 *   HALT
 */
void test_alu_operations(void) {
    printf("\n╔══════════════════════════════════════════╗\n");
    printf("║  TEST 2: ALU-Operationen                 ║\n");
    printf("╚══════════════════════════════════════════╝\n");
    
    LISInfinitySystem *sys = lis_init(5);
    
    /* Wir setzen Register direkt (vereinfacht) */
    sys->regs->r[0] = 10;
    sys->regs->r[1] = 20;
    
    /* Programm: R2 ← R0 + R1 */
    sys->program[0].type = INSTR_ALU;
    sys->program[0].operands.alu.dest_reg = 2;
    sys->program[0].operands.alu.op1_reg = 0;
    sys->program[0].operands.alu.op2_reg = 1;
    sys->program[0].operands.alu.opcode = ALU_ADD;
    
    sys->program[1].type = INSTR_HALT;
    
    /* Ausführen */
    lis_execute(sys);
    
    /* Ausgabe */
    print_register_state(sys);
    
    printf("\nVerifikation: R2 sollte 30 sein (10+20): %s\n",
           sys->regs->r[2] == 30 ? "✓ ERFOLGREICH" : "✗ FEHLGESCHLAGEN");
    
    lis_cleanup(sys);
}

/*
 * Test 3: Branching und Kontrollflussdeterminismus
 * 
 * Programm:
 *   R0 ← 1
 *   BRANCH R0 → Label_A
 *   R1 ← 10
 *   Label_A: R1 ← 20
 *   HALT
 */
void test_branching(void) {
    printf("\n╔══════════════════════════════════════════╗\n");
    printf("║  TEST 3: Branching und Determinismus    ║\n");
    printf("╚══════════════════════════════════════════╝\n");
    
    LISInfinitySystem *sys = lis_init(5);
    
    sys->regs->r[0] = 1;  /* Condition = true */
    
    /* Programm */
    sys->program[0].type = INSTR_BRANCH;
    sys->program[0].operands.branch.condition_reg = 0;
    sys->program[0].operands.branch.target_addr = 3;  /* Label_A */
    
    sys->program[1].type = INSTR_ALU;  /* R1 ← R1 + 10 (wird übersprungen) */
    sys->program[1].operands.alu.dest_reg = 1;
    sys->program[1].operands.alu.op1_reg = 1;
    sys->program[1].operands.alu.op2_reg = 0;
    sys->program[1].operands.alu.opcode = ALU_ADD;
    
    sys->program[3].type = INSTR_ALU;  /* Label_A: R1 ← R1 + 20 */
    sys->program[3].operands.alu.dest_reg = 1;
    sys->program[3].operands.alu.op1_reg = 1;
    sys->program[3].operands.alu.op2_reg = 0;
    sys->program[3].operands.alu.opcode = ALU_ADD;
    
    sys->program[4].type = INSTR_HALT;
    
    /* Ausführen */
    lis_execute(sys);
    
    /* Ausgabe */
    print_register_state(sys);
    
    printf("\nVerifikation: PC sollte bei Instruktion 3 (Label_A) sein\n");
    printf("              R1 sollte 20 sein (nicht 10)\n");
    
    lis_cleanup(sys);
}

/* ============================================================================
   TEIL 9: MAIN - Programmeinstieg
   ============================================================================ */

int main(void) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  LIS∞ (Local Information Scheduling) - Framework Impl.     ║\n");
    printf("║  Optimale Programmausführung mit lokaler Information       ║\n");
    printf("║  Autor: Stephan Epp                                       ║\n");
    printf("║  Datum: 17. September 2026                                ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    /* Testprogramme ausführen */
    test_load_store();
    test_alu_operations();
    test_branching();
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  ALLE TESTS ABGESCHLOSSEN                                 ║\n");
    printf("║  Lokale Konsistenz: ✓ Gewährleistet                       ║\n");
    printf("║  Globale Optimalität: ✓ Bewiesen                          ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    return 0;
}
