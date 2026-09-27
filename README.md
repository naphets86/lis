# Wie die CPU mit Peripheriegeräten kommuniziert

## Grundprinzip: Daten werden gepulst

Über eine **Kommunikationsleitung** werden Daten gepulst. Ein Puls kann **0 oder 1** sein, was auf verschiedenen Abstraktionsebenen bedeutet:
- Elektrisch: **Low (0V)** oder **High (3,3V)** (oder 5V, je nach Mikrocontroller)
- Digital: **0** oder **1**

Jeder Puls wird über eine Kommunikationsleitung geleitet.

## Was ist ein Bus?

Eine **Kommunikationsleitung wird Bus genannt**, wenn sie für die Datenübertragung zwischen der CPU und Peripheriegeräten genutzt wird.

Es gibt **drei Arten von Kommunikationsleitungen**:

### 1. Der Adressbus
Der Adressbus ist eine Gruppe von Kommunikationsleitungen, auf denen Adressen gepulst werden. Jede Leitung wird zum Pulsen eines Bits der Adresse verwendet. Zusammen werden diese Leitungen verwendet, um die vollständige Adresse eines Registers oder Speicherorts zu leiten.

Beispiel: Ein 16-Bit-Adressbus besteht aus 16 Kommunikationsleitungen. Wenn eine Adresse gepulst wird, werden über die 16 Leitungen (durch ihre Pulse: 0V oder 3,3V) eine eindeutige Adresse von 0x0000 bis 0xFFFF geleitet.

### 2. Der Datenbus
Der Datenbus ist eine Gruppe von Kommunikationsleitungen, auf denen Daten gepulst werden. Jede Leitung wird zum Pulsen eines Bits der Daten verwendet.

Beispiel: Ein 8-Bit-Datenbus besteht aus 8 Kommunikationsleitungen. Wenn Daten gepulst werden, werden über die 8 Leitungen zusammen ein Datenbyte (0x00 bis 0xFF) geleitet.

### 3. Der Kontrollbus
Der Kontrollbus ist eine Gruppe von Kommunikationsleitungen, auf denen Kontrollsignale gepulst werden. Diese Signale steuern den Datenfluss:
- **Read-Signal**: Sagt dem Peripheriegerät, dass die CPU Daten lesen möchte
- **Write-Signal**: Sagt dem Peripheriegerät, dass die CPU Daten schreiben möchte
- **Enable-Signal**: Aktiviert oder deaktiviert das Peripheriegerät

## Der Kommunikationsprozess

Wenn die CPU in ein Register eines Peripheriegeräts schreiben möchte:

1. **Adresse pulsen**: Die CPU pulst auf dem Adressbus die Adresse des Ziel-Registers (0V und 3,3V in der richtigen Sequenz auf den 16 Leitungen)

2. **Daten pulsen**: Die CPU pulst auf dem Datenbus die zu schreibenden Daten (0V und 3,3V in der richtigen Sequenz auf den 8 Leitungen)

3. **Kontrollsignal pulsen**: Die CPU pulst auf dem Kontrollbus ein Write-Signal (HIGH auf der Write-Leitung)

4. **Peripheriegerät reagiert**: Das Peripheriegerät überwacht alle drei Busse gleichzeitig. Wenn es die Adresse erkennt, die für es bestimmt ist, nimmt es die gepulsten Daten entgegen und führt die entsprechende Aktion aus.

## Beispiel: LED mit GPIO anschalten

Szenario: Die CPU möchte einen GPIO-Pin auf HIGH setzen, um eine LED anzuschalten.

1. Die CPU pulst auf dem Adressbus die GPIO-Kontrollregister-Adresse (z.B. 0x4001)
2. Die CPU pulst auf dem Datenbus den Wert 0x01 (das Bit für Pin 0, das 1 bedeutet)
3. Die CPU pulst auf dem Kontrollbus das Write-Signal
4. Das GPIO-Peripheriegerät erkennt seine Adresse, liest die gepulsten Daten (0x01), und setzt den Pin 0 auf HIGH (3,3V)
5. Die LED, die an Pin 0 angeschlossen ist, leuchtet auf

Die CPU spricht nicht direkt mit der LED. Sie pulst Daten über die Busse, und das Peripheriegerät führt die Arbeit aus.

## Warum diese Architektur?

Diese Architektur ermöglicht es einer CPU, viele verschiedene Peripheriegeräte über wenige Gruppen von Kommunikationsleitungen zu steuern. Jedes Peripheriegerät:
- Überwacht alle Busse ständig
- Wartet auf seine spezifische Adresse
- Reagiert auf gepulste Daten und Kontrollsignale
- Ignoriert Pulse, die nicht für es bestimmt sind

Dies ist effizienter, als hätte die CPU für jedes Peripheriegerät separate Leitungen.

## Zusammenfassung

**Daten werden über Kommunikationsleitungen gepulst. Diese Leitungen werden Busse genannt. Die drei Arten von Bussen – Adressbus, Datenbus, Kontrollbus – arbeiten zusammen, um die CPU mit Peripheriegeräten zu verbinden.**
