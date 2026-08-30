# 🚀 AliothX Kernel — EEVDF & CASS Edition

<p align="center">
  <img src="https://img.shields.io/badge/Kernel-Linux%204.19.325--cip135--st19-blue?style=for-the-badge&logo=linux" alt="Kernel Version">
  <img src="https://img.shields.io/badge/Device-POCO%20F3%20%7C%20Redmi%20K40%20(alioth)-brightgreen?style=for-the-badge&logo=xiaomi" alt="Device">
  <img src="https://img.shields.io/badge/SoC-Qualcomm%20Snapdragon%20870%20(SM8250--AC)-red?style=for-the-badge&logo=qualcomm" alt="SoC">
  <img src="https://img.shields.io/badge/Scheduler-EEVDF%20%2B%20CASS-orange?style=for-the-badge" alt="Scheduler">
  <img src="https://img.shields.io/badge/Toolchain-Android%20Clang%2021.0%20(ThinLTO%2BPGO)-purple?style=for-the-badge" alt="Toolchain">
  <img src="https://img.shields.io/badge/Status-Stable%20%26%20Active-success?style=for-the-badge" alt="Status">
</p>

---

## 📖 Visão Geral / Overview

**AliothX** é um kernel de alta performance customizado para os smartphones **POCO F3**, **Redmi K40** e **Xiaomi Mi 11X** (`alioth` / `aliothin`), equipados com a plataforma móvel **Qualcomm Snapdragon 870 5G (SM8250-AC)**.

O objetivo principal deste projeto é trazer inovações de ponta das versões mais recentes do kernel Linux upstream (Linux 6.6+) diretamente para a base **Linux 4.19**, combinando o pioneirismo do escalonador **EEVDF (Earliest Eligible Virtual Deadline First)** com o esquema de escalonamento consciente de capacidade **CASS**, otimizações de concorrência e compressão moderna.

---

## ⚡ Principais Tecnologias e Funcionalidades Portadas

### 🎯 1. Escalonador EEVDF (Earliest Eligible Virtual Deadline First)
Substituição completa do escalonador CFS (*Completely Fair Scheduler*) pelo moderno algoritmo **EEVDF**, introduzido no Linux 6.6 pelo mantenedor Peter Zijlstra:
- **Elegibilidade Temporal (*Virtual Eligibility*):** Uma tarefa só pode ser selecionada pela CPU se for "elegível", ou seja, se seu tempo virtual decorrido não tiver ultrapassado o tempo virtual médio da fila.
- **Prazos Virtuais (*Virtual Deadlines*):** Em vez de ordenar tarefas puramente por tempo virtual acumulado (`vruntime`), o EEVDF calcula prazos com base na latência requerida e no peso da tarefa:
  $$d_i = v_i + rac{q_i}{w_i}$$
  Tarefas interativas de UI (como eventos de toque, renderização de quadros e rolagem) recebem prazos menores e são despachadas imediatamente.
- **Proteção de Lag (*vlag* / *vprot*):** Mecanismo que protege tarefas sensíveis a latência contra a apropriação indevida de tempo por tarefas de processamento massivo em segundo plano (*batch*), **eliminando os micro-engasgos (*micro-stutter*) a 120 Hz**.
- **Fatias de Tempo Dinâmicas (*Dynamic Slice Sizing*):** O escalonador ajusta de forma contínua o tamanho das fatias de execução com base no consumo real de CPU.

---

### 🧠 2. CASS (Capacity Aware Schedule Scheme) & Energy Model
O EEVDF foi harmonizado com a topologia assimétrica tri-cluster do processador Snapdragon 870:
- **Cluster Silver (Little):** 4x Kryo 585 Silver (Cortex-A55 @ 1.80 GHz) — Foco em eficiência máxima para tarefas de fundo e I/O.
- **Cluster Gold (Big):** 3x Kryo 585 Gold (Cortex-A77 @ 2.42 GHz) — Foco em cargas sustentadas e aplicativos do dia a dia.
- **Cluster Prime (Super):** 1x Kryo 585 Prime (Cortex-A77 @ 3.20 GHz) — Foco em picos de desempenho, jogos pesados e renderização crítica.
- **Integração com o Energy Model:** Implementação de `arch_scale_cpu_capacity` e `arch_scale_min_freq_capacity`, permitindo que o EEVDF calcule o custo energético e a capacidade real de cada núcleo antes de tomar decisões de migração.

---

### 🔄 3. Modern Workqueue Concurrency Engine
Substituição da lógica legada de concorrência do Workqueue pelas rotinas modernas:
- **`wq_worker_sleeping()` & `wq_worker_running()`:** Integradas diretamente no ciclo principal de troca de contexto `__schedule()`.
- **Prevenção de Deadlocks:** Eliminação de travamentos circulares em kworkers durante montagem de partições UFS, inicialização de drivers de GPU/Display e transições de sono profundo.

---

### 🛡️ 4. Ciclo de Vida de Processos & RCU Refcounting
- **Harmonização do `finish_task_switch` com `task_struct`:** Correção fundamental no descarte de cotas de escalonamento para tarefas em estado `TASK_DEAD` utilizando `put_task_struct_rcu_user(prev)` conforme as especificações do Linux 4.19.
- **Segurança no `wait4` / `wait_task_zombie`:** Salvaguarda contra dereferências de ponteiro nulo em credenciais (`real_cred`), permitindo que serviços do userspace (como o `ueventd` e o `/init` do Android) colham processos filhos sem risco de *kernel panic* ou bootloop.

---

### 🎮 5. Estabilidade de GPU Adreno & Reguladores de Clock
- **Recuperação Graciosa da GPU Adreno:** Em `drivers/gpu/msm/adreno_dispatch.c`, eliminamos o `BUG_ON(ret)` original da Qualcomm. Se a GPU Adreno 650 sofrer um travamento ou *timeout* em jogos pesados sob estresse térmico, o despachante adia a retentativa graciosamente em vez de reiniciar o aparelho para o modo Recovery.
- **Tolerância a Timeout em GDSC (500 µs):** Em `drivers/clk/qcom/gdsc-regulator.c`, o tempo limite de espera de transição de estado dos reguladores de clock foi elevado de 100 µs para 500 µs, alinhando com o subsistema principal e prevenindo congelamentos de tela e câmera.

---

### 📦 6. Compressão e Otimizações de Sistema
- **ZSTD 1.5.7:** Mecanismo de compressão Zstandard atualizado para zRAM e swap, oferecendo taxas de compressão superiores e descompressão ultrarrápida.
- **DroidSpaces:** Isolamento e particionamento de namespaces de memória para multitarefa robusta.
- **CPU Input Boost Otimizado:** Resposta ao toque com latência ultra-baixa, elevando frequências na medida exata do toque sem desperdício de energia.
- **Uclamp Assist:** Otimização para taxas de atualização dinâmicas e telas de 120 Hz.
- **Compilador Clang 21.0.0 (`r563880c`):** Compilação com otimizações em tempo de linkagem LTO completa (*ThinLTO*), perfilamento orientado por feedback (*PGO*), pós-linkagem *BOLT* e otimização por aprendizado de máquina *MLGO*.

---

## 📥 Download da Versão Estável (Universal ZIP)

O AliothX adota o formato universal **AnyKernel3**, garantindo compatibilidade automática com **qualquer ROM** (LineageOS, EvolutionX, crDroid, PixelOS, HyperOS, etc.), adaptando-se em tempo real tanto a **Header v2** quanto a **Header v3**.

👉 [**Baixar a Release Mais Recente (GitHub Releases)**](https://github.com/otaviomorais/AliothX/releases/latest)

| Arquivo | Compatibilidade | Como Instalar |
| :--- | :--- | :--- |
| **`AliothX-DroidSpaces-alioth-*.zip`** | **100% Universal** (Header v2 e v3) | Flashear via **TWRP / OrangeFox Recovery** ou **Kernel Flasher** |

---

## 📲 Como Instalar

### Método Recomendado: Via Recovery (TWRP / OrangeFox) ou Kernel Flasher
1. Baixe o pacote `.zip` da release mais recente.
2. Reinicie o smartphone no modo Recovery (**Volume Mais** + **Botão Power**) ou abra o app **Kernel Flasher** com root.
3. Selecione o arquivo `.zip` e confirme o flash. O instalador inteligente AnyKernel3 identificará automaticamente o formato de boot da sua ROM e atualizará o kernel de forma limpa e segura.
4. Reinicie o sistema (*Reboot System*). Não é necessário limpar dados nem cache!

---

## 🔍 Como Verificar o Escalonador no Aparelho

Você pode checar se o kernel e o EEVDF estão rodando através do **Termux** ou **ADB**:

```bash
# Verificar a versão e commit do kernel
cat /proc/version

# Se possuir acesso root, visualize as métricas e fatias do EEVDF por CPU
su -c "cat /proc/sched_debug | head -n 40"
```

---

## 🤝 Créditos e Agradecimentos

- **Linus Torvalds, Peter Zijlstra & Desenvolvedores do Kernel Linux** pelo desenvolvimento do EEVDF.
- **Google & Android Open Source Project (AOSP)** pela base e toolchains Clang/LLVM.
- **Qualcomm Technologies, Inc. & CodeAurora (CAF)** pelo BSP do Snapdragon 870.
- **Comunidade AliothX / POCO F3** pelo suporte, testes e feedback contínuo.
