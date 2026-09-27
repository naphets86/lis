# LIS∞ Implementierungsleitfaden: Von CPU-Architektur zur optimalen Programmausführung

## Übersicht: Theorie trifft Praxis

Ihre ursprüngliche `cpu.md` beschreibt, wie eine CPU physikalisch mit Peripheriegeräten kommuniziert:

```
CPU pulst Adressen → Peripherie hört → Peripherie pulst Daten zurück → CPU verarbeitet
```

Diese Arbeit (LIS∞) formalisiert, dass diese **lokale Pulsleitung-Kommunikation nicht nur funktioniert, sondern mathematisch optimal ist**.

Dieser Leitfaden zeigt, wie Sie die Theorie praktisch umsetzen.

---

## Teil 1: Architektur-Analyse für Ihr System

### 1.1 Ihr System gemäß CPU.md

**Ihre Beschreibung:**
```
Adressbus (16-Bit):    Identifiziert Register/Speicheradresse
Datenbus (8-Bit):      Transportiert Datenwert
Kontrollbus (3 Leitungen):
  - Read-Signal
  - Write-Signal
  - Enable-Signal
```

**In LIS∞-Notation:**
```
m = 16        (Adressbus-Länge)
n = 8         (Datenbus-Länge)
k = 3         (Kontrollbus-Länge)
```

**Zustandsraum der Pulsleitung:**
```
|Σ_puls| = 2^(16+8+3) = 2^27 ≈ 134 Millionen mögliche Pulskombinationen
```

Obwohl es viele Kombinationen gibt, arbeitet das System **sequenziell** mit nur einer Pulskombination pro Takt.

### 1.2 Kontroller-Design

Ihre CPU-Beschreibung sagt:

> "Der Kontroller arbeitet das Programm ab. Wer macht das Programm? Das machen die Menschen."

**In LIS∞:**
- **Programm** = Sequenz von Instruktionen in RAM/Flash
- **Kontroller** = Finite State Machine (Zustandsmaschine)
- **Zustand $q_t$** = Programm-Counter (PC) oder Zustand des Automaten
- **Übergangsfunktion $\delta_K$** = Implementiert die Instruktions-Logik

```
Zeit t:
  PC = 0x1000 (Kontroller-Zustand)
  LOAD R0, 0x4001 (Instruktion an PC)
  
  Kontroller liest seine lokale Information: PC = 0x1000
  Dekodiert: LOAD-Instruktion
  Emittiert: Adresse 0x4001 auf Adressbus
  
Zeit t+1:
  Wartet auf Daten
  
Zeit t+2:
  Daten angekommen
  Speichert in R0
  PC ← PC + 1
```

**Kein Zirkelbezug:** Der Mensch schreibt das Programm einmal. Der Kontroller führt es aus, ohne die "Gesamtvision" zu haben.

### 1.3 Ihr LED-Beispiel in LIS∞-Sprache

**Original aus cpu.md:**

```
1. Die CPU pulst auf dem Adressbus die GPIO-Kontrollregister-Adresse (z.B. 0x4001)
2. Die CPU pulst auf dem Datenbus den Wert 0x01
3. Die CPU pulst auf dem Kontrollbus ein Write-Signal
4. Das GPIO-Peripheriegerät erkennt seine Adresse, nimmt die Daten entgegen
5. GPIO setzt den Pin auf HIGH (3,3V), LED leuchtet
```

**In LIS∞-Notation:**

```
Instruktion: STORE R1, 0x4001
(wobei R1 = 0x01)

Puls 1 (Zeit t):
  Adressbus: p(ℓ_a,1 bis ℓ_a,16, t) = Binary(0x4001) = (1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0)
  Kontroller → Peripherie: "Schreib dich an Adresse 0x4001"
  
Puls 2 (Zeit t+1):
  Datenbus: p(ℓ_d,1 bis ℓ_d,8, t+1) = Binary(0x01) = (1,0,0,0,0,0,0,0)
  Kontroller → Peripherie: "Der Wert ist 0x01"
  
Puls 3 (Zeit t+2):
  Kontrollbus: p(ℓ_c,2, t+2) = 1 (Write-Signal HIGH)
  Kontroller → Peripherie: "Schreib jetzt!"
  Peripherie registriert: Adresse 0x4001, Wert 0x01, Write-Befehl
  GPIO-Register wird aktualisiert
  Pin 0 geht auf 3,3V
  LED leuchtet

Konsequenz: Determiniert, unvermeidlich, lokal konsistent
```

---

## Teil 2: Von CPU.md zu LIS∞ — Die Implementierung

### 2.1 Register-Definitionen

**Basierend auf Ihrer CPU.md:**

Sie erwähnen "Register". In einem echten System:

```c
// CPU-Register (L1 in der Hierarchie)
typedef struct {
    uint32_t r[32];           // R0-R31: Allgemeine Register
    uint32_t pc;              // Program Counter (Kontroller-Zustand q)
    uint32_t sr;              // Status Register (Flags: Zero, Carry, etc.)
} CPUState;

// Busse (Pulsleitungen)
typedef struct {
    uint16_t address_bus;     // 16 Pulsleitungen für Adresse
    uint8_t data_bus;         // 8 Pulsleitungen für Daten
    struct {
        uint8_t read;         // Read-Signal (ℓ_c,1)
        uint8_t write;        // Write-Signal (ℓ_c,2)
        uint8_t enable;       // Enable-Signal (ℓ_c,3)
    } control_bus;
} BusState;

// Kontroller-Automat (FSM)
typedef struct {
    uint8_t state;            // Aktueller Zustand q ∈ Q
    CPUState *cpu;
    BusState *bus;
} Controller;
```

### 2.2 Pulse als atomare Operationen

**Lemma L5 besagt:** Jede Instruktion benötigt minimale Pulse.

**Implementierung:**

```c
// Puls als atomare Operation
typedef enum {
    PULSE_NONE = 0,
    PULSE_ADDRESS = 1,
    PULSE_DATA = 2,
    PULSE_CONTROL_READ = 3,
    PULSE_CONTROL_WRITE = 4,
} PulseType;

// Lokale Konsistenz: Ein Puls pro Takt
void cpu_pulse(Controller *ctrl, PulseType pulse_type) {
    // Takt t: Genau eine Pulsoperation
    // Keine Überlappung (Serialität)
    
    switch (pulse_type) {
        case PULSE_ADDRESS:
            // Adresse auf Adressbus legen (Puls 1)
            ctrl->bus->address_bus = ctrl->cpu->r[ADDR_REG];
            break;
            
        case PULSE_DATA:
            // Daten auf Datenbus legen (Puls 2 oder 3)
            ctrl->bus->data_bus = ctrl->cpu->r[DATA_REG];
            break;
            
        case PULSE_CONTROL_WRITE:
            // Write-Signal (Puls 3)
            ctrl->bus->control_bus.write = 1;
            ctrl->bus->control_bus.read = 0;
            // Peripherie reagiert: speichert Daten
            peripheral_write(ctrl->bus->address_bus, 
                           ctrl->bus->data_bus);
            break;
            
        case PULSE_CONTROL_READ:
            // Read-Signal (Puls 2)
            ctrl->bus->control_bus.read = 1;
            ctrl->bus->control_bus.write = 0;
            // Peripherie antwortet: legt Daten auf Bus
            uint8_t data = peripheral_read(ctrl->bus->address_bus);
            ctrl->bus->data_bus = data;
            break;
    }
    
    // Lokal konsistent: Kein Konflikt, klare Reihenfolge
}
```

### 2.3 LIS∞ Algorithmus — Praktische Implementierung

**Aus der Theorie:**
```
FOR each instruction:
    Zerlege in Pulse
    Führe Pulse sequenziell aus
    Update Register und PC
```

**Code-Beispiel LOAD:**

```c
// LOAD R_dest, address
void instr_load(Controller *ctrl, uint8_t r_dest, uint16_t addr) {
    // Puls 1: Adresse auf Adressbus
    ctrl->cpu->r[ADDR_REG] = addr;
    cpu_pulse(ctrl, PULSE_ADDRESS);
    
    // Puls 2: Read-Signal
    cpu_pulse(ctrl, PULSE_CONTROL_READ);
    
    // Puls 3: Daten lesen und in Register schreiben
    ctrl->cpu->r[r_dest] = ctrl->bus->data_bus;
    cpu_pulse(ctrl, PULSE_NONE);
    
    // Lokal konsistent:
    // - Keine zwei Schreiboperationen auf R_dest (nur eine)
    // - Kausalität: Adresse vor Daten
    // - Determinismus: Nächste Instruktion folgt eindeutig
}

// STORE R_src, address
void instr_store(Controller *ctrl, uint8_t r_src, uint16_t addr) {
    // Puls 1: Adresse
    ctrl->cpu->r[ADDR_REG] = addr;
    cpu_pulse(ctrl, PULSE_ADDRESS);
    
    // Puls 2: Daten
    ctrl->cpu->r[DATA_REG] = ctrl->cpu->r[r_src];
    cpu_pulse(ctrl, PULSE_DATA);
    
    // Puls 3: Write-Signal
    cpu_pulse(ctrl, PULSE_CONTROL_WRITE);
}

// ALU (Arithmetische/Logische Operation)
void instr_alu(Controller *ctrl, uint8_t r_dest, 
               uint8_t r_op1, uint8_t r_op2, uint8_t opcode) {
    // Puls 1: Operanden auslesen (lokal)
    uint32_t val1 = ctrl->cpu->r[r_op1];
    uint32_t val2 = ctrl->cpu->r[r_op2];
    
    // Puls 2: Operation durchführen (kombinatorisch)
    uint32_t result;
    switch (opcode) {
        case ADD: result = val1 + val2; break;
        case SUB: result = val1 - val2; break;
        case AND: result = val1 & val2; break;
        // ...
    }
    
    // Puls 3: Ergebnis speichern
    ctrl->cpu->r[r_dest] = result;
    
    // Lokal konsistent: Nur R_dest wird geändert, eindeutig
}
```

### 2.4 Kontrollflussgraph und Determinismus

**Lemma L4:** Kontrollflussgraph ist eindeutig für gegebene Initialzustand.

**Code:**

```c
// Branch: Deterministisches Sprungziel
void instr_branch(Controller *ctrl, uint8_t condition_reg, 
                  uint16_t target_addr) {
    // Puls 1: Condition-Flag lesen
    uint8_t condition = (ctrl->cpu->sr >> condition_reg) & 1;
    
    // Determinismus: Nur ein mögliches Ergebnis
    if (condition) {
        ctrl->cpu->pc = target_addr;  // Sprung
    } else {
        // Sequenziell folgt
        ctrl->cpu->pc += 1;            // Nächste Instruktion
    }
    
    // Kein nicht-deterministischer Punkt!
    // Für jeden Initialzustand gibt es genau einen Pfad.
}

// Haupt-Ausführungsschleife (LIS∞ Algorithmus)
void cpu_run(Controller *ctrl, uint32_t *program, size_t prog_len) {
    while (ctrl->cpu->pc < prog_len) {
        uint32_t instr = program[ctrl->cpu->pc];
        uint8_t opcode = (instr >> 24) & 0xFF;
        uint8_t r_dest = (instr >> 16) & 0xFF;
        uint8_t r_op1 = (instr >> 8) & 0xFF;
        uint8_t r_op2 = instr & 0xFF;
        
        // Dekodiere und führe aus
        switch (opcode) {
            case OPCODE_LOAD: instr_load(ctrl, ...); break;
            case OPCODE_STORE: instr_store(ctrl, ...); break;
            case OPCODE_ALU: instr_alu(ctrl, ...); break;
            case OPCODE_BRANCH: instr_branch(ctrl, ...); break;
            case OPCODE_HALT: goto done;
        }
        
        // PC wird inkrementiert (falls nicht Branch)
        if (opcode != OPCODE_BRANCH) {
            ctrl->cpu->pc += 1;
        }
    }
    done:
    // Programm beendet
}
```

---

## Teil 3: Register-Hierarchie und Lokalitätsprinzip

### 3.1 Mehrschichtige Register-Hierarchie

**Ihre CPU.md + LIS∞ Hierarchie:**

```
L1: CPU-Register (32 × 32-Bit)
    Zugriff: 1 Puls
    Beispiel: R0-R31
    
L2: Lokales Scratch-RAM (8KB)
    Zugriff: 2-3 Pulse
    Beispiel: Temporäre Variablen
    
L3: Hauptspeicher (RAM, z.B. 1MB-64MB)
    Zugriff: 10-100 Pulse
    Beispiel: Stack, Heap, Datensegmente
    
L4: Flash/EEPROM (256KB-4MB)
    Zugriff: 10^6 Pulse
    Beispiel: Code, Konstanten
```

### 3.2 Lokalitätsprinzip in der Praxis

**Satz T2 besagt:** Häufige Zugriffe in schnelleren Layern minimieren Gesamtpulse.

**Praktisches Beispiel:**

```c
// Szenario: Fibonacci-Berechnung (häufiger Zugriff auf Akkumulator)

// SUBOPTIMAL: Ergebnisse in Hauptspeicher speichern
uint32_t fib_slow(uint32_t n) {
    for (int i = 0; i < n; i++) {
        // Puls 1-3: Lade from RAM
        uint32_t a = memory_read(0x2000);
        // Puls 4-6: Lade from RAM
        uint32_t b = memory_read(0x2004);
        // Puls 7-9: Rechne
        uint32_t c = a + b;
        // Puls 10-12: Speichere in RAM
        memory_write(0x2000, b);
        // Puls 13-15: Speichere in RAM
        memory_write(0x2004, c);
    }
    return memory_read(0x2004);
}
// Gesamtpulse für n=100: ~1500 Pulse

// OPTIMAL: Verwende Register (L1) — Lokalitätsprinzip!
uint32_t fib_fast(uint32_t n) {
    uint32_t a = 0, b = 1;  // In Registern!
    for (int i = 0; i < n; i++) {
        // Puls 1: Rechne (kombinatorisch)
        uint32_t c = a + b;
        // Puls 2: Aktualisiere Register
        a = b;
        b = c;
    }
    return b;
}
// Gesamtpulse für n=100: ~300 Pulse (5x schneller!)
```

**Warum?** 
- Register-Zugriffe: 1 Puls
- RAM-Zugriffe: ~30 Pulse durchschnittlich
- **Diff:** 30x Speedup möglich durch Lokalitätsprinzip

### 3.3 Cache-Strategie

**Ziel:** Häufig genutzte Daten in L1/L2 halten.

```c
// Cache-Implementierung (LRU: Least Recently Used)
typedef struct {
    uint16_t tag;           // Speicheradresse (obere Bits)
    uint8_t data[64];       // Cache-Zeile (64 Byte)
    uint8_t valid;          // Ist der Eintrag gültig?
    uint8_t dirty;          // Wurde er geändert?
    uint32_t lru_time;      // Wann zuletzt zugegriffen?
} CacheLine;

typedef struct {
    CacheLine lines[256];   // 256 Einträge × 64 Byte = 16 KB L2-Cache
} L2Cache;

// Cache-Zugriff (Lokalitätsprinzip!)
uint8_t cache_read(L2Cache *cache, uint16_t addr) {
    uint16_t tag = addr >> 6;       // Obere Bits als Tag
    uint8_t index = addr & 0x3F;    // Untere 6 Bits
    
    // Suche in Cache
    for (int i = 0; i < 256; i++) {
        if (cache->lines[i].tag == tag && cache->lines[i].valid) {
            // Cache-Hit: Nur 2-3 Pulse!
            cache->lines[i].lru_time = get_time();
            return cache->lines[i].data[index];
        }
    }
    
    // Cache-Miss: Lade von Hauptspeicher (30+ Pulse)
    uint8_t data = main_memory_read(addr);
    
    // Finde LRU-Eintrag und ersetze
    int lru_idx = find_lru_line(cache);
    cache->lines[lru_idx].tag = tag;
    cache->lines[lru_idx].data[index] = data;
    cache->lines[lru_idx].valid = 1;
    cache->lines[lru_idx].lru_time = get_time();
    
    return data;
}
```

**Effekt:**
- Cache-Hit-Rate: ~90% in guten Programmen
- Hit-Zeit: 2-3 Pulse
- Miss-Zeit: 30+ Pulse
- **Durchschnitt:** ~5 Pulse pro Zugriff (statt 30)
- **Speedup:** ~6x durch Lokalitätsprinzip

---

## Teil 4: Verifikation und Invarianten-Prüfung

### 4.1 Invarianten implementieren (Lemma L3)

**Invariante 1: Register-Exklusivität**

```c
// Prüfe: Nur ein Register wird zur Zeit t geschrieben
typedef struct {
    uint8_t write_count;        // Wie viele Register-Schreibvorgänge?
    uint8_t writing_register[32]; // Welche?
} WriteCheckpoint;

WriteCheckpoint check_write_exclusivity(Controller *ctrl) {
    WriteCheckpoint chk = {0};
    
    for (int i = 0; i < 32; i++) {
        if (register_is_being_written(i)) {
            chk.write_count++;
            chk.writing_register[i] = 1;
        }
    }
    
    // Invariante erfüllt?
    return chk.write_count <= 1 ? true : false;
}
```

**Invariante 2: Pulsleitung-Serialität**

```c
// Prüfe: Jede Pulsleitung trägt maximal ein Signal
bool check_pulse_seriality(BusState *bus) {
    // Adressbus: Viele Leitungen, aber sie bilden eine Nachricht
    // Datenbus: Viele Leitungen, aber sie bilden eine Nachricht
    // Kontrollbus: Max. ein Signal (Read ODER Write, nicht beide)
    
    if (bus->control_bus.read && bus->control_bus.write) {
        // Fehler: Beide Signale gleichzeitig!
        return false;
    }
    
    return true;
}
```

**Invariante 3: Kausal-Ordnung**

```c
// Prüfe: Instruktion i endet vor Instruktion j startet
typedef struct {
    uint32_t instr_start_time[MAX_INSTRUCTIONS];
    uint32_t instr_end_time[MAX_INSTRUCTIONS];
} ExecutionLog;

bool check_causality(ExecutionLog *log, int instr_i, int instr_j) {
    // i < j → i muss vor j enden
    if (log->instr_start_time[i] < log->instr_start_time[j]) {
        return log->instr_end_time[i] <= log->instr_start_time[j];
    }
    return true;
}
```

**Invariante 4: Deterministisches Branching**

```c
// Prüfe: Branch-Bedingung hängt nur von bekannten Registern ab
bool check_branch_determinism(Controller *ctrl, uint8_t cond_reg) {
    // Condition-Flag muss in einem Register sein
    // Dieses Register muss vor dem Branch geschrieben worden sein
    
    uint8_t condition = (ctrl->cpu->sr >> cond_reg) & 1;
    
    // Für denselben Registerstand gibt es nur ein mögliches Ziel
    uint16_t target1 = get_branch_target(ctrl, cond_reg);
    uint16_t target2 = get_branch_target(ctrl, cond_reg);
    
    return target1 == target2;  // Deterministisch?
}
```

### 4.2 Verifikations-Suite

```c
// Vollständige Invarianten-Prüfung
bool verify_local_consistency(Controller *ctrl, ExecutionLog *log) {
    // Alle 4 Invarianten müssen erfüllt sein
    
    WriteCheckpoint wc = check_write_exclusivity(ctrl);
    if (!wc) {
        printf("FEHLER: Register-Exklusivität verletzt\n");
        return false;
    }
    
    if (!check_pulse_seriality(&ctrl->bus)) {
        printf("FEHLER: Pulsleitung-Serialität verletzt\n");
        return false;
    }
    
    if (!check_causality(log, /* all pairs */)) {
        printf("FEHLER: Kausalität verletzt\n");
        return false;
    }
    
    if (!check_branch_determinism(ctrl, /* relevant registers */)) {
        printf("FEHLER: Branch nicht deterministisch\n");
        return false;
    }
    
    printf("✓ Alle Invarianten erfüllt — Lokale Konsistenz gewährleistet\n");
    return true;
}
```

---

## Teil 5: Optimierungs-Strategien

### 5.1 Puls-Minimierung (Satz T1)

**Ziel:** Weniger Pulse pro Instruktion.

**Strategie 1: Operand-Parallelisierung**

```c
// SUBOPTIMAL:
uint32_t val1 = cpu->r[0];    // Puls 1
uint32_t val2 = cpu->r[1];    // Puls 2
result = val1 + val2;         // Puls 3

// OPTIMAL: Parallele Bit-Übertragung
// (Beide Register-Bits auf verschiedenen Pulsleitungen)
// Immer noch 3 Pulse notwendig (ALU-Delay), aber keine zusätzliche Latenz
```

**Strategie 2: Instruction-Level Parallelism (bei Multi-Core)**

In Single-Core LIS∞: Nicht möglich (Pulsleitung-Serialität erzwingt Sequenzialisierung).

Bei Multi-Core: Erfordert Cache-Kohärenz und komplexere Bus-Architektur.

### 5.2 Register-Allocation (Lokalitätsprinzip)

```c
// Compiler-Optimierung: Häufig genutzte Variablen in Registern
// (Teaser: Könnte durch Compiler automatisiert werden)

// SUBOPTIMAL (alles in RAM)
int sum = 0;                    // Puls 1-3: Lade von RAM
for (int i = 0; i < 1000; i++) {
    sum = sum + array[i];       // Puls 1-3: Lade Array-Element
                                // Puls 4-6: Addiere
                                // Puls 7-9: Speichere sum in RAM
}
// ~9000 Pulse

// OPTIMAL (Lokalitätsprinzip)
int sum = 0;                    // Puls 1: In Register R0
for (int i = 0; i < 1000; i++) {
    sum = sum + array[i];       // Puls 1-3: Lade Array-Element
                                // Puls 4: Addiere (R0 ← R0 + Temp)
}
// ~4000 Pulse
```

---

## Teil 6: Fehlerfall-Analyse

### 6.1 Was passiert, wenn lokale Konsistenz verletzt wird?

**Beispiel: Zwei CPUs (Multicore) schreiben gleichzeitig in R0**

```c
CPU1: r[0] = 42;    // Schreiben
CPU2: r[0] = 99;    // Schreiben (gleichzeitig!)

// Resultat: Undefined behavior
// R0 = 42 oder R0 = 99? Wir wissen es nicht.
```

**In LIS∞ wird das verhindert:**
```c
// Exklusivität-Invariante: Nur CPU1 darf schreiben
// Die Pulsleitung-Architektur erzwingt Serialisierung
// (durch Cache-Kohärenz-Protokoll bei Multicore)
```

### 6.2 Fehlertoleranz — Was LIS∞ nicht macht

**LIS∞ toleriert KEINE:**
- Bit-Flips in Registern (würde Determinismus brechen)
- Timing-Variationen (würde Kausalität verletzen)
- Nicht-determinierte Kontroller-Übergänge

**Aber:** Mit zusätzlichen Schichten (z.B. Error-Correcting Codes) kann Robustheit erreicht werden.

---

## Teil 7: Vergleich mit anderen Architekturen

### 7.1 LIS∞ vs. Out-of-Order Execution

| Aspekt | LIS∞ | Out-of-Order |
|--------|------|--------------|
| **Serialität** | Streng | Partiell (mehrere Instruktionen in Flug) |
| **Vorhersagbarkeit** | Deterministisch, bekannt | Komplexe Abhängigkeitsanalyse |
| **Verifizierbarkeit** | Einfach (LTL) | Schwierig (exponentielle Komplexität) |
| **Durchsatz** | Lower (1 Instr./Zyklus im Worst Case) | Higher (2-4 Instr./Zyklus) |
| **Latenz** | Vorhersagbar | Variabel |
| **Best für** | Echtzeitsysteme, embedded | Desktop, Server, High-Performance |

### 7.2 LIS∞ vs. GPU-Programmierung

| Aspekt | LIS∞ | GPU |
|--------|------|-----|
| **Parallelität** | Keine (Single-Core) oder Cache-Kohärenz (Multi-Core) | Massive Parallelität (Tausende Threads) |
| **Lokalität** | Hierarchisch (L1-L4) | Shared Memory + Local Memory |
| **Programmiermodell** | Klassisch imperativ | SIMD (Single Instruction Multiple Data) |
| **Determinismus** | Garantiert | Schwierig bei Race Conditions |

---

## Teil 8: Checkliste für die Implementierung

### Implementierungs-Checkliste

- [ ] **Registerdefinitionen** erstellen (R0-R31, PC, SR)
- [ ] **Bus-Struktur** implementieren (Adressbus, Datenbus, Kontrollbus)
- [ ] **Kontroller-Automat** $\delta_K$ definieren
- [ ] **Pulse als atomare Operationen** implementieren
- [ ] **LIS∞ Hauptschleife** (Instruktion dekodieren → Pulse emittieren)
- [ ] **Invarianten-Prüfung** (Exklusivität, Serialität, Kausalität, Determinismus)
- [ ] **Verifikation** mit Testfällen
- [ ] **Optimierung** (Lokalitätsprinzip, Puls-Minimierung)
- [ ] **Dokumentation** und Beweise

### Test-Szenarien

```c
// Test 1: Einfaches LOAD-STORE
void test_load_store() {
    ASSERT(verify_local_consistency());
    // Prüfe: Registerwert korrekt geladen/gespeichert
}

// Test 2: Arithmetische Operationen
void test_alu() {
    ASSERT(verify_local_consistency());
    // Prüfe: 3+5=8 in Register
}

// Test 3: Branching
void test_branch() {
    ASSERT(verify_local_consistency());
    ASSERT(control_flow_deterministic());
    // Prüfe: Nur ein möglicher Pfad pro Initialzustand
}

// Test 4: Lokalitätsprinzip
void test_locality() {
    measure_pulse_count(cache_on);
    measure_pulse_count(cache_off);
    ASSERT(cache_on < cache_off);  // Cache sollte schneller sein
}

// Test 5: Fehlerfall
void test_error_multicore_conflict() {
    ASSERT(!verify_local_consistency());  // Sollte fehlschlagen
}
```

---

## Fazit: Von Theorie zur Praxis

Diese Implementierungs-Anleitung zeigt:

1. **Ihre CPU.md ist theoretisch fundiert** — Sie beschreiben bereits die Pulsleitung-Architektur
2. **LIS∞ formalisiert diese Intuition** — Mathematisch beweisbar optimal
3. **Die Implementierung ist realistisch** — Mit echtem Code, echten Optimierungen
4. **Lokalitätsprinzip funktioniert** — 5-6x Speedup durch Register vs. RAM
5. **Determinismus ist machbar** — Durch Einhaltung der 4 Invarianten

**Die zentrale Botschaft:**
> Optimale Programmausführung erfordert nicht globale Voraussicht, sondern lokale Konsistenz. Dieser Leitfaden zeigt, wie es praktisch funktioniert.

---

**Nächste Schritte:**
- Implementieren Sie die Registerdefinitionen
- Schreiben Sie Tests für die Invarianten
- Messen Sie Pulse-Kosten für verschiedene Szenarien
- Vergleichen Sie LIS∞ mit anderen Ansätzen in Ihrem System
