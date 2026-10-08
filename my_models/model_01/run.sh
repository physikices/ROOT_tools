#!/usr/bin/env bash
# Gera as duas tabelas e a figura (PDF, PNG e TeX).
set -e
cd "$(dirname "$0")"
python3 gen_tabelas.py --outdir .
for ext in pdf png tex; do
  root -l -b -q "plota_modelo.C(\"tabela_PT.dat\",\"tabela_W.dat\",\"fig_modelo.${ext}\")"
done
