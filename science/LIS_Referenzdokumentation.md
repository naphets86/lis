# LIS∞: Referenzdokumentation — Formeln, Theoreme und Beweise

## Inhaltsverzeichnis

1. [Definitionen (Formalismen)](#definitionen)
2. [Lemmata (Kleinere Sätze)](#lemmata)
3. [Theoreme (Hauptsätze)](#theoreme)
4. [Korollare (Folgerungen)](#korollare)
5. [Beweise — Zusammenfassung](#beweise)
6. [Formeln und Mathematische Ausdrücke](#formeln)
7. [Algorithmen-Pseudocode](#algorithmen)
8. [Schnellreferenz-Tabellen](#schnellreferenz)

---

## Definitionen

### D1: Pulsleitung

Eine **Pulsleitung** $\ell$ ist ein Übertragungsmedium für binäre Signale:

$$p(\ell, t) \in \{0, 1\}$$

wobei:
- $p(\ell, t) = 0 \Rightarrow$ Low (0V)
- $p(\ell, t) = 1 \Rightarrow$ High (3.3V oder 5V)

**Eigenschaften:**
- Unidirektional (eine Richtung)
- Zeitlich (abhängig von $t$)
- Deterministisch (eindeutiger Zustand)

**Beispiel:** Adressbus mit $m=16$ Pulsleitungen: $\ell_{a,1}, \ell_{a,2}, \ldots, \ell_{a,16}$

---

### D2: Lokale Information

Lokale Information $I_{\text{lok}}(t)$ existiert an genau einem Ort zu genau einer Zeit:

$$I_{\text{lok}}(t) \in I_{\text{reg}}(t) \cup I_{\text{puls}}(t) \cup I_{\text{ctrl}}(t)$$

wobei disjunkt (no overlap):
- $I_{\text{reg}}(t)$ = Registerinhalte zur Zeit $t$
- $I_{\text{puls}}(t)$ = Pulsleitungszustände zur Zeit $t$
- $I_{\text{ctrl}}(t)$ = Kontroller-Zustand zur Zeit $t$

**Zentrales Postulat:** Keine Komponente hat gleichzeitigen Zugriff auf alle Informationen.

---

### D3: Bus-System

Ein **Bus-System** $\mathcal{B}$ ist ein geordnetes Tripel:

$$\mathcal{B} = (\mathcal{A}, \mathcal{D}, \mathcal{C})$$

wobei:

#### Adressbus $\mathcal{A}$:
$$\mathcal{A} = (\ell_{a,1}, \ell_{a,2}, \ldots, \ell_{a,m}), \quad m = 16 \text{ (typisch)}$$

Nachricht zur Zeit $t$:
$$\mathbf{a}(t) = (p(\ell_{a,1}, t), \ldots, p(\ell_{a,m}, t)) \in \{0,1\}^m$$

**Interpretiert als:** Adresse $\text{addr} = \sum_{i=1}^{m} p(\ell_{a,i}, t) \cdot 2^{i-1} \in [0, 2^m-1]$

#### Datenbus $\mathcal{D}$:
$$\mathcal{D} = (\ell_{d,1}, \ldots, \ell_{d,n}), \quad n = 8 \text{ (typisch)}$$

Nachricht zur Zeit $t$:
$$\mathbf{d}(t) = (p(\ell_{d,1}, t), \ldots, p(\ell_{d,n}, t)) \in \{0,1\}^n$$

**Interpretiert als:** Datenwert $\text{data} = \sum_{i=1}^{n} p(\ell_{d,i}, t) \cdot 2^{i-1} \in [0, 2^n-1]$

#### Kontrollbus $\mathcal{C}$:
$$\mathcal{C} = (\ell_{c,1}, \ell_{c,2}, \ell_{c,3})$$

- $\ell_{c,1}$ = Read-Signal
- $\ell_{c,2}$ = Write-Signal
- $\ell_{c,3}$ = Enable-Signal

Nachricht: $\mathbf{c}(t) = (p(\ell_{c,1}, t), p(\ell_{c,2}, t), p(\ell_{c,3}, t)) \in \{0,1\}^3$

---

### D4: Register-Zustand

Der **Register-Zustand** des Systems zur Zeit $t$:

$$\mathcal{R}(t) = (r_1(t), r_2(t), \ldots, r_s(t))$$

wobei:
- $s$ = Anzahl der Register (z.B. 32)
- $r_i(t) \in \{0,1\}^w$ = Wert des $i$-ten Registers (Wortbreite $w$, z.B. 32 Bit)

**Zustandsraum:**
$$\mathcal{R}(t) \in \mathbb{R} = (\{0,1\}^w)^s$$

**Größe:** $|\mathbb{R}| = 2^{s \cdot w}$ mögliche Zustände.

---

### D5: Kontroller-Automat

Der **Kontroller-Automat** $\mathcal{K}$ ist ein deterministiertes endliches Zustandssystem:

$$\mathcal{K} = (\mathbb{Q}, \Sigma_{\text{puls}}, \delta_{\mathcal{K}}, q_0)$$

wobei:
- $\mathbb{Q}$ = endliche Menge von Zuständen (z.B. $|\mathbb{Q}| = 2^{10}$ für 10-Bit PC)
- $\Sigma_{\text{puls}} = \{0,1\}^{m+n+k}$ = Eingabealphabet (alle Pulsleitung-Kombinationen)
  - $m+n+k = 16+8+3 = 27$ (typisch)
- $\delta_{\mathcal{K}} : \mathbb{Q} \times \Sigma_{\text{puls}} \to \mathbb{Q}$ = **determinierte** Übergangsfunktion
- $q_0 \in \mathbb{Q}$ = Initialzustand (z.B. PC = 0)

**Operationsweise:**
```
Takt t:
  1. Lese alle Pulsleitung-Werte: input(t) ∈ Σ_puls
  2. Berechne Übergangsfunktion: q(t+1) = δ_K(q(t), input(t))
  3. Emittiere neue Steuersignale basierend auf q(t+1)
```

---

### D6: Konsequenzen-Determinismus

Ein System erfüllt **Konsequenzen-Determinismus** wenn:

$$\forall t \in \mathbb{N}: (\mathcal{R}(t), q(t), \mathbf{a}(t), \mathbf{d}(t), \mathbf{c}(t)) \Rightarrow (\mathcal{R}(t+1), q(t+1), \ldots) \text{ eindeutig}$$

Formaler, mit der Übergangsfunktion $\Gamma$:

$$\Gamma : (\mathcal{R}, q, \text{Puls}) \to (\mathcal{R}', q')$$

ist eine **partielle Funktion** (für alle gültigen Eingaben definiert) und **eindeutig** (nur eine Ausgabe).

**Intuition:** Wie die Sonne immer scheint (unvermeidlich), führt jeder konsistente Zustand zu genau einem Nachfolgezustand.

---

### D7: Lokale Konsistenz

Ein Register-Zustand $\mathcal{R}(t)$ ist **lokal konsistent** wenn:

1. **Eindeutigkeit des Kontroller-Zustands:** 
   $$q(t) \text{ ist eindeutig aus } (\mathcal{R}(0), \text{Pulshistorie}[0, t)) \text{ bestimmbar}$$

2. **Kausalität (Happens-Before Relation):**
   $$\text{Wenn } r_i \text{ vor } r_j \text{ geschrieben wurde, dann } \text{age}(r_i, t) < \text{age}(r_j, t)$$

3. **Exklusivität (No Write-Write Conflict):**
   $$\forall i, j \in [1, s]: i \neq j \Rightarrow \text{keine zwei Instruktionen schreiben gleichzeitig in } r_i \text{ und } r_j$$

**Mathematische Notation:**

Sei $W(t)$ die Menge aller Schreiboperationen bis Zeit $t$. Lokale Konsistenz erfordert:
- Für alle $(i, t_1), (i, t_2) \in W(t)$ mit $t_1 < t_2$: Es gibt keine konkurrierende Operation zu demselben Register
- Die Ordnung auf Schreiboperationen ist total (linear) und respektiert Kausalität

---

### D8: Programmabarbeitung

Eine **Programmabarbeitung** ist eine endliche oder unendliche Sequenz von Zustandsübergängen:

$$(q_0, \mathcal{R}(0)) \xrightarrow{\text{Puls}_1} (q_1, \mathcal{R}(1)) \xrightarrow{\text{Puls}_2} \cdots$$

wobei:
- Jeder Übergang ist durch Konsequenzen-Determinismus eindeutig bestimmt
- Jeder Puls $\text{Puls}_k$ ist ein Bit-Wechsel auf einer oder mehreren Pulsleitungen
- Die Sequenz respektiert lokale Konsistenz

**Terminierung:** Die Abarbeitung endet, wenn die Instruktion HALT erreicht wird, oder läuft unendlich (Schleife).

---

## Lemmata

### L1: Lokale Konsistenz impliziert Determinismus

**Lemma (L1):** Ist der Register-Zustand $\mathcal{R}(t)$ lokal konsistent, dann existiert ein **eindeutiger** Nachfolgezustand $\mathcal{R}(t+1)$.

**Beweis-Struktur (4 Schritte):**

1. **Eindeutigkeit von $q(t)$:** Aus der Kausalität folgt $q(t)$ eindeutig (kein nichtdeterminierter Kontroller-Übergangspunkt)

2. **Eindeutigkeit der Kontroller-Eingabe:** Alle Pulsleitung-Werte sind eindeutig, da sie von bekannten Registerwerten abhängen

3. **Eindeutigkeit von $\delta_{\mathcal{K}}$:** Die Übergangsfunktion ist deterministisch (Definition)

4. **Eindeutigkeit von $\mathcal{R}(t+1)$:** Alle Register-Änderungen folgen aus eindeutigen Steuersignalen, keine Konflikte (Exklusivität)

**Konklusion:** $\mathcal{R}(t+1)$ existiert und ist eindeutig.

---

### L2: Konsequenzen-Kette (Transitive Eindeutigkeit)

**Lemma (L2):** Ist ein Zustand $(\mathcal{R}(t), q(t))$ in Konsequenzen-Determinismus und lokaler Konsistenz, dann ist jeder Zustand $(\mathcal{R}(k), q(k))$ für $k > t$ eindeutig durch Initialzustand und Pulshistorie bestimmt.

$$\forall t \in \mathbb{N}: (\mathcal{R}(t), q(t)) = \Gamma^t(\mathcal{R}(0), q_0, \text{Pulse}[0, t))$$

**Beweis:** Induktion über $t$ mit L1 als Induktionsschritt.

**Intuition:** Kein Zustand ist mehrdeutig. Die gesamte Abarbeitung ist eine determinierte Sequenz.

---

### L3: Lokale Konsistenz in LIS∞ (Invarianten)

**Lemma (L3):** Während der Ausführung von LIS∞ gelten:

1. **Register-Exklusivität:** $\forall t: |\{i : r_i \text{ wird zur Zeit } t \text{ geschrieben}\}| \leq 1$

2. **Pulsleitung-Serialität:** $\forall t, \ell: p(\ell, t)$ trägt höchstens eine Nachricht

3. **Kausal-Ordnung:** $\text{start}(i) < \text{start}(j) \Rightarrow \text{Ende}(i) < \text{Ende}(j)$

4. **Deterministisches Branching:** Branch-Bedingungen hängen nur von bekannten Registerwerten ab

**Beweis:** Struktur des LIS∞-Algorithmus garantiert jede Invariante durch Design.

---

### L4: Kontrollflussdeterminismus

**Lemma (L4):** Jedes in LIS∞ ausgeführte Programm mit eindeutigem Initialzustand $(\mathcal{R}_0, q_0)$ folgt einem **eindeutigen Kontrollflussgraph**.

**Formalisierung:** Der Kontrollflussgraph $G_{\text{exec}} = (V, E_{\text{exec}})$ ist für eine gegebene Initialzustand einelementig zu durchlaufen:

$$\forall v_i, v_j \in G_{\text{exec}}: \text{Es gibt genau einen Pfad von } v_i \text{ zu } v_j$$

**Beweis:** Nach L2 ist jeder Registerzustand eindeutig. Branch-Entscheidungen folgen aus eindeutigen Registerwerten, daher ist jeder Sprung deterministisch.

**Konsequenz:** Keine non-deterministic choice points im Kontrollflussgraph.

---

### L5: Pulsoptimalität

**Lemma (L5):** Jede Instruktion in LIS∞ wird mit **minimaler Pulsanzahl** ausgeführt.

**Spezifikation:**
- LOAD, STORE, ALU: 3 Pulse (Minimum der Pulsleitung-Architektur)
- BRANCH: 1 Puls
- HALT: 0 Pulse

**Beweis:** Physikalische Constraints der Serialität:
- Adresse muss übertragen werden (1 Puls parallel)
- Kontrollsignal muss gesetzt werden (1 Puls parallel)
- Daten müssen übertragen werden (1 Puls parallel)
- Weniger ist unmöglich ohne Funktion zu beeinträchtigen

---

## Theoreme

### T1: Globale Optimalität aus lokaler Konsistenz

**Theorem (T1) — Hauptsatz:**

Für ein System $\mathcal{S} = (\mathcal{R}, \mathcal{B}, \mathcal{K}, \text{Program})$, das erfüllt:

1. Alle Register-Zustandsübergänge sind lokal konsistent (L3)
2. Der Kontroller arbeitet deterministisch ($\delta_{\mathcal{K}}$ ist Funktion)
3. Datenfluss folgt Adressbus-Adressierung (keine Bypass)
4. Programmausführung respektiert Konsequenzen-Kette (L2)

**Behauptung:** Es existiert **keine alternative Abarbeitungssequenz** $\pi'$ mit weniger Pulsen, die denselben Initialzustand zu einem äquivalenten Endzustand führt.

**Formalisiert:**

Sei $\pi = (\text{Puls}_1, \ldots, \text{Puls}_T)$ die unter obigen Bedingungen ausgeführte Pulssequenz mit Länge $T$.

$$\forall \pi' = (\text{Puls}_1', \ldots, \text{Puls}_{T'}') : T' < T \Rightarrow \text{Endzustand}(\pi') \not\equiv \text{Endzustand}(\pi)$$

**Beweis-Struktur (4 Schritte):**

1. **Unausweichlichkeit der Registeränderungen** (Beweisschritt 1):
   - Jede Registeränderung folgt aus lokaler Konsistenz
   - Keine kann umgangen werden, ohne Funktionalität zu verfälschen

2. **Minimalität der Pulsoperationen** (Beweisschritt 2):
   - Jeder Puls ist das Minimum zur Erreichung des nächsten konsistenten Zustands
   - Keine Parallelität in der Pulsleitung-Architektur
   - Sequenzialisierung ist erzwungen

3. **Kausalitätskette** (Beweisschritt 3):
   - Nach L2: Jeder Zustand ist eindeutig durch bisherige Pulse bestimmt
   - Lokale Konsistenz verbietet, dass Sequenzen verkürzt werden

4. **Konsequenz-Determinismus erzwingt Minimalität** (Beweisschritt 4):
   - Wenn ein Puls existiert, war er notwendig
   - Die Übergangsfunktion $\delta_{\mathcal{K}}$ ist optimal definiert

**Konklusion:** $\pi$ ist **optimal** in der Anzahl der Pulsoperationen.

---

### T2: Lokalitätsprinzip ist optimal

**Theorem (T2):**

Wenn eine Programmabarbeitung das **Lokalitätsprinzip** respektiert (häufige Zugriffe in schnelleren Register-Layern), dann ist die Gesamtpulsanzahl minimal über alle möglichen Speicherzugriffsmuster mit denselben Ein-/Ausgaben.

**Mathematische Formulierung:**

Sei $W = \{w_1, \ldots, w_k\}$ die Menge aller Speicherzugriffe. Für jeden Zugriff $w_i$ und Layer $L_j$:

$$C(w_i, L_j) = \text{Pulskosten für Zugriff } w_i \text{ aus Layer } L_j$$

Die Gesamtpulsanzahl:
$$T_{\text{gesamt}} = \sum_{w_i \in W} C(w_i, L_{j(w_i)})$$

wobei $L_{j(w_i)}$ der Layer ist, auf dem $w_i$ residiert.

**Behauptung:** $T_{\text{gesamt}}$ ist minimal wenn:
$$f_{\text{häufig}}(w_i) \text{ hoch} \Rightarrow L_{j(w_i)} \text{ schnell (niedrige } j)$$

**Beweis:** Direkter Vergleich: Häufige Zugriffe in $L_1$ (1 Puls) vs. $L_3$ (50 Pulse) führt offensichtlich zu weniger Gesamtpulsen.

---

### T3: Hierarchische Konsequenzen

**Theorem (T3):**

In einem System mit Register-Hierarchie $\mathcal{H} = (L_1, L_2, L_3, L_4)$ gelten:

1. **Exklusivität:** Ein Wert existiert zu einer Zeit in genau einem Layer
   $$\forall \text{value}: |\{L : \text{value} \in L \text{ zur Zeit } t\}| \leq 1$$

2. **Konsistenz:** Wenn $L_i$ aus $L_{i+1}$ kopiert, muss $L_i$ den Wert halten solange er benötigt wird

3. **Optimalität:** Minimale Gesamtpulse ⟺ Lokalitätsprinzip (Satz T2)

**Konsequenzen:**
- Cache-Kohärenz ist nicht optional, sondern mathematisch erzwungen
- Register-Hierarchie ist optimal strukturiert
- Verstöße gegen diese Regeln führen zu nicht-optimaler Ausführung

---

## Korollare

### K1: Universelle Optimalität

**Korollar (K1):**

Alle Systeme, die den vier Bedingungen von T1 genügen, haben **global optimale Ausführung** hinsichtlich:
- Zyklusanzahl
- Register-Zugriffe
- Datenbahn-Aktivierung

---

### K2: Skalierbarkeit

**Korollar (K2):**

Wenn ein Programm mit $N$ Instruktionen die Bedingungen erfüllt, dann:

$$T_{\text{puls}} = \Theta(N)$$

Die Ausführungszeit skaliert linear mit Programmgröße, unabhängig von Register-Hierarchie-Tiefe (unter Normalfall-Bedingungen).

---

### K3: Verifikationsfreundlichkeit

**Korollar (K3):**

LIS∞-Systeme sind vollständig verifizierbar durch temporale Logik, da:
- Konsequenzen-Determinismus guarantee safety und liveness properties
- Keine non-deterministic choice points existieren
- Invarianten sind formal beweisbar

---

## Beweise — Zusammenfassung

| Lemma/Satz | Beweistyp | Länge | Schwierigkeit |
|-----------|-----------|--------|----------------|
| L1 | Konstruktiv (4 Schritte) | 1 Seite | Mittel |
| L2 | Induktion | 1 Seite | Leicht |
| L3 | Struktur-Analyse | 0,5 Seite | Leicht |
| L4 | Direkter Beweis | 0,5 Seite | Leicht |
| L5 | Kontra-Position | 1 Seite | Mittel |
| T1 | Kontra-Position + Kausalität | 2 Seiten | Schwer |
| T2 | Vergleich | 0,5 Seite | Mittel |
| T3 | Direkt | 1 Seite | Mittel |

---

## Formeln und Mathematische Ausdrücke

### Größen und Komplexität

#### Register-Zustandsraum
$$|\mathbb{R}| = 2^{s \cdot w}$$
wobei $s$ = Anzahl Register, $w$ = Wortbreite.

**Beispiel:** 32 Register mit 32 Bit: $2^{32 \times 32} = 2^{1024}$ mögliche Zustände

#### Pulsleitung-Eingabealphabet
$$|\Sigma_{\text{puls}}| = 2^{m+n+k}$$
wobei $m+n+k$ = Anzahl Pulsleitung-Pins (z.B. 27)

#### Zeitkomplexität
Für Programm mit $N$ Instruktionen:
$$T_{\text{puls}} = 3N_{\text{ALU}} + 3N_{\text{MEM}} + N_{\text{BRANCH}} + \Theta(1)$$

Durchschnittlich: $T_{\text{puls}} \approx 3N$

### Übergangsfunktion

#### Kontroller-Übergangsfunktion
$$q(t+1) = \delta_{\mathcal{K}}(q(t), \mathbf{input}(t))$$

wobei $\mathbf{input}(t) = (\mathbf{a}(t), \mathbf{d}(t), \mathbf{c}(t))$

#### Register-Updatefunktion
$$r_i(t+1) = \begin{cases}
\mathbf{d}(t) & \text{falls } p(\ell_{c,2}, t) = 1 \text{ (Write-Signal)} \\
r_i(t) & \text{sonst (keine Änderung)}
\end{cases}$$

### Lokalitätsprinzip — Pulskosten

#### Zugriff aus verschiedenen Layern
| Layer | Pulskosten | Beispiel |
|-------|-----------|---------|
| L1 (CPU-Register) | 1 | R0 ← 42 |
| L2 (Cache) | 2-3 | Memory ← R0 (Cache-Hit) |
| L3 (RAM) | 10-100 | Memory ← R0 (RAM) |
| L4 (Disk/Flash) | 10^6 | Load from Disk |

#### Optimale Allokation
Häufigkeit-Gewichteter Zugriff:
$$\text{Gesamtpulse} = \sum_{w \in W} f(w) \cdot C(w, L_{\text{gewählt}})$$

Minimierung durch Lokalitätsprinzip:
$$\min \sum_{w} f(w) \cdot C(w, L(w))$$

---

## Algorithmen

### Alg. 1: LIS∞ Programmabarbeitungs-Algorithmus

```pseudocode
PROCEDURE LIS∞(R0, q0, Program, B):
    R ← R0                          // Initialzustand Register
    q ← q0                          // Initialzustand Kontroller
    t ← 0                           // Zykluszähler
    halted ← false
    
    WHILE NOT halted DO
        cmd ← q                     // Kommandozustand
        instr ← Program[cmd]        // Nächste Instruktion
        
        CASE instr OF
            
            LOAD(r_dest, addr):
                // Puls 1: Adresse auf Adressbus
                pulse_address(addr)
                // Puls 2: Read-Signal
                pulse_control(READ=1)
                // Puls 3: Daten lesen und in Register schreiben
                R[r_dest] ← read_data()
                t ← t + 3
            
            STORE(r_src, addr):
                // Puls 1: Adresse
                pulse_address(addr)
                // Puls 2: Daten auf Datenbus
                pulse_data(R[r_src])
                // Puls 3: Write-Signal
                pulse_control(WRITE=1)
                t ← t + 3
            
            ALU(r_dest, r_op1, r_op2, opcode):
                // Puls 1: Operanden lesen
                val1 ← R[r_op1]
                val2 ← R[r_op2]
                // Puls 2: Operation durchführen
                result ← ALU_execute(val1, val2, opcode)
                // Puls 3: Ergebnis speichern
                R[r_dest] ← result
                t ← t + 3
            
            BRANCH(condition, target):
                // Puls 1: Condition prüfen und springe
                IF condition(R) THEN
                    q ← q + target
                ELSE
                    q ← q + 1
                ENDIF
                t ← t + 1
                CONTINUE  // Keine PC-Inkrementierung
            
            HALT:
                halted ← true
        
        END CASE
        
        IF NOT (BRANCH) THEN
            q ← q + 1               // PC inkrementieren
        ENDIF
    
    END WHILE
    
    RETURN R                        // Finaler Register-Zustand
END PROCEDURE
```

### Alg. 2: Lokale Konsistenz-Prüfung

```pseudocode
PROCEDURE check_local_consistency(R, q, history):
    // Prüfe 4 Invarianten
    
    // 1. Eindeutigkeit q(t)
    IF NOT is_unique_controller_state(q, history) THEN
        RETURN false
    ENDIF
    
    // 2. Kausalität
    IF NOT respects_causality(R, history) THEN
        RETURN false
    ENDIF
    
    // 3. Register-Exklusivität
    IF has_write_write_conflict(R, history) THEN
        RETURN false
    ENDIF
    
    // 4. Determinsitisches Branching
    IF NOT deterministic_branches(R) THEN
        RETURN false
    ENDIF
    
    RETURN true
END PROCEDURE
```

---

## Schnellreferenz-Tabellen

### Tabelle 1: Instruktionstypen und Pulskosten

| Instruktion | Operation | Pulse | Bedingung |
|------------|-----------|-------|-----------|
| LOAD R_d, addr | Speicher → Register | 3 | Adresse + Read + Daten |
| STORE R_s, addr | Register → Speicher | 3 | Adresse + Daten + Write |
| ALU R_d, R_s1, R_s2, op | Berechnung | 3 | Operanden + Op + Schreiben |
| BRANCH cond, target | Sprung | 1 | Bedingung + PC-Update |
| HALT | Stop | 0 | Programm Ende |
| NOP | Nichtstun | 1 | Dummy-Operation |

### Tabelle 2: Register-Hierarchie

| Layer | Typ | Anzahl | Pulskosten | Latenz | Größe |
|--------|-----|--------|-----------|--------|--------|
| L1 | CPU-Register | 32 | 1 | ~0,5ns | 256B |
| L2 | L1-Cache | 8K-32K | 2-3 | ~1ns | 32KB |
| L3 | L2-Cache | 256K-1M | 5-10 | ~5ns | 512KB |
| L4 | Hauptspeicher (RAM) | Gigabytes | 50-100 | ~100ns | 4GB-16GB |
| L5 | Flash/SSD | Terabytes | 10^5-10^6 | ~100µs | 256GB-2TB |

### Tabelle 3: Komplexitäts-Kategorisierung

| Konzept | Zeitkomplexität | Speicherkomplexität | Verifizierbarkeit |
|---------|-----------------|--------|-------|
| LIS∞ (Single-Instr.) | O(1) | O(1) | Hoch |
| LIS∞ (N Instruktionen) | O(N) | O(N) | Hoch |
| Globalplanung | O(N log N) oder O(2^N) | O(N^2) | Mittel |
| Out-of-Order Execution | O(N log N) | O(N) | Niedrig |

---

## Notations-Referenz

| Symbol | Bedeutung | Beispiel |
|--------|-----------|----------|
| $\ell$ | Pulsleitung | $\ell_{a,5}$ = Adressbus-Leitung 5 |
| $p(\ell, t)$ | Puls-Wert zu Zeit t | $p(\ell_{a,5}, t) = 1$ (High) |
| $\mathcal{R}(t)$ | Register-Zustand | $(r_1, \ldots, r_s)$ zur Zeit t |
| $q(t)$ | Kontroller-Zustand | Position im Programm |
| $\mathbf{a}(t)$ | Adresse-Nachricht | 16-Bit Vektor |
| $\mathbf{d}(t)$ | Daten-Nachricht | 8-Bit Vektor |
| $\delta_{\mathcal{K}}$ | Übergangsfunktion | $q' = \delta_{\mathcal{K}}(q, \text{input})$ |
| $\Gamma$ | System-Übergangsfunktion | $(\mathcal{R}', q') = \Gamma(\mathcal{R}, q, ...)$ |
| $\pi$ | Pulssequenz | $(\text{Puls}_1, \ldots, \text{Puls}_T)$ |
| $T_{\text{puls}}$ | Pulsanzahl | Total in Abarbeitung |

---

## Häufig gestellte Fragen (zur Referenz)

**Q1: Warum ist lokale Information besser als globale Planung?**
A: Sie sind komplementär. Globale Planung schreibt den Algorithmus, lokale Ausführung implementiert ihn optimal.

**Q2: Kann LIS∞ parallelisiert werden?**
A: Nein, die Pulsleitung-Serialität verbietet echte Parallelität. Bei Multicore ist zusätzliche Komplexität erforderlich.

**Q3: Ist LIS∞ schneller als Out-of-Order Execution?**
A: Nein, OoOE kann schneller sein in einigen Szenarien. LIS∞ ist einfacher zu verifizieren und vorhersagbar.

**Q4: Wie verifiziere ich LIS∞ Systeme?**
A: Mittels temporaler Logik (LTL) und model checking (UPPAAL, TLA+).

**Q5: Gilt LIS∞ auch für Quantencomputer?**
A: Noch unbekannt. Quantenmechanische Lokalität ist andersartig als klassische.

---

**Ende der Referenzdokumentation**
