#!/usr/bin/env python3
"""Gera as duas tabelas que alimentam plota_modelo.C.

  tabela_PT.dat  -> painel esquerdo  (dsigma/dPT [nb/GeV] vs PT [GeV])
  tabela_W.dat   -> painel direito   (sigma [nb] vs W_gp [GeV])

Formato (formato longo, uma linha por bin/ponto):
  serie  xlo  xhi  y  eyl  eyh

  - curvas teoricas: histograma em degraus, y no bin [xlo, xhi]; eyl = eyh = 0
  - dados:           ponto em (xlo+xhi)/2, barra x = (xhi-xlo)/2, barras y = eyl/eyh (absolutas)
  - banda:           duas series (band_lo, band_hi), preenchidas entre si pela macro

ATENCAO: os valores abaixo foram LIDOS A OLHO da figura modelo; sao numeros
ilustrativos para reproduzir o layout, nao resultados fisicos. Para usar numeros
reais, substitua as listas por suas saidas (HO2.6.7 / PYTHIA8.310) e pelos dados do H1.

Uso: python3 gen_tabelas.py [--outdir .]
"""
import argparse
import os


def bins(edges):
    return list(zip(edges[:-1], edges[1:]))


def curva(edges, ys):
    assert len(edges) - 1 == len(ys)
    return [(lo, hi, y, 0.0, 0.0) for (lo, hi), y in zip(bins(edges), ys)]


def dados(edges, ys, errs):
    assert len(edges) - 1 == len(ys) == len(errs)
    return [(lo, hi, y, e, e) for (lo, hi), y, e in zip(bins(edges), ys, errs)]


def escreve(caminho, titulo, colunas_x, blocos):
    with open(caminho, "w") as f:
        f.write(f"# {titulo}\n")
        f.write(f"# x = {colunas_x}\n")
        f.write("# serie  xlo  xhi  y  eyl  eyh\n")
        for nome, linhas in blocos:
            for lo, hi, y, el, eh in linhas:
                f.write(f"{nome:<10s} {lo:8.3f} {hi:8.3f} {y:12.5e} {el:12.5e} {eh:12.5e}\n")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--outdir", default=".")
    a = ap.parse_args()
    os.makedirs(a.outdir, exist_ok=True)

    # ---------------- Tabela 1: dsigma/dPT [nb/GeV] x PT [GeV] ----------------
    ept = [float(i) for i in range(11)]  # 10 bins de 1 GeV: 0..10
    ts1_ho = [1.55, 1.50, 1.12, 0.68, 0.36, 0.17, 0.075, 0.030, 0.011, 0.0040]
    fator_py = [1.02, 1.10, 1.15, 1.20, 1.22, 1.25, 1.30, 1.35, 1.40, 1.50]
    ts1_py = [v * f for v, f in zip(ts1_ho, fator_py)]
    s0_ho = [0.022, 0.013, 0.0075, 0.0042, 0.0021, 0.0010, 0.00040, 0.00018, 0.00006, 0.000018]
    s0_py = [0.024, 0.052, 0.055, 0.042, 0.025, 0.012, 0.0045, 0.0018, 0.0006, 0.0002]

    # dados H1: 9 pontos, binagem variavel (centro +- meia largura)
    c = [0.70, 1.40, 2.10, 2.80, 3.50, 4.30, 5.50, 6.90, 8.60]
    hw = [0.35, 0.35, 0.35, 0.35, 0.35, 0.45, 0.75, 0.60, 1.40]
    y_all = [1.40, 2.10, 1.65, 1.10, 0.62, 0.29, 0.055, 0.0072, 0.0015]
    rel = [0.10, 0.08, 0.08, 0.08, 0.10, 0.12, 0.15, 0.20, 0.70]
    sub = [0.98, 0.98, 0.97, 0.96, 0.95, 0.93, 0.88, 0.82, 0.60]  # "b -> J/psi subtraido"

    def pontos(ys, rels):
        return [(ci - h, ci + h, y, y * r, y * r) for ci, h, y, r in zip(c, hw, ys, rels)]

    t1 = [
        ("ts1_ho", curva(ept, ts1_ho)),
        ("ts1_py", curva(ept, ts1_py)),
        ("s0_ho", curva(ept, s0_ho)),
        ("s0_py", curva(ept, s0_py)),
        ("h1_all", pontos(y_all, rel)),
        ("h1_bsub", pontos([y * s for y, s in zip(y_all, sub)], rel)),
    ]
    escreve(os.path.join(a.outdir, "tabela_PT.dat"),
            "Painel esquerdo: dsigma/dPT [nb/GeV]", "PT [GeV]", t1)

    # ---------------- Tabela 2: sigma [nb] x W [GeV] ----------------
    ew = [float(w) for w in range(60, 241, 20)]  # 9 bins de 20 GeV: 60..240
    w_ts1_ho = [14.0, 17.0, 19.5, 22.0, 25.0, 27.5, 29.5, 31.0, 32.0]
    w_ts1_tune = [19.5, 24.5, 28.0, 31.0, 34.0, 36.5, 38.5, 40.0, 41.5]
    w_s0_ho = [2.1, 2.4, 2.7, 3.0, 3.3, 3.55, 3.8, 4.1, 4.4]
    w_s0_tune = [18.5, 23.5, 28.0, 32.0, 35.5, 38.5, 41.0, 43.0, 45.0]
    w_band_lo = [4.0] * 9
    w_band_hi = w_ts1_ho

    # dados H1 EPJC 68 (2010): 8 pontos
    ed = [60, 80, 100, 120, 140, 160, 180, 210, 240]
    d_y = [23.0, 24.0, 24.0, 30.0, 35.0, 30.0, 31.0, 33.0]
    d_e = [4.0, 4.5, 5.0, 5.0, 5.5, 6.0, 6.0, 7.0]
    # o primeiro ponto (W=69) tem barra x de 60 a 78
    d_bins = list(zip(ed[:-1], ed[1:]))
    d_bins[0] = (60.0, 78.0)
    d_lin = [(lo, hi, y, e, e) for (lo, hi), y, e in zip(d_bins, d_y, d_e)]

    t2 = [
        ("ts1_ho", curva(ew, w_ts1_ho)),
        ("ts1_tune", curva(ew, w_ts1_tune)),
        ("s0_ho", curva(ew, w_s0_ho)),
        ("s0_tune", curva(ew, w_s0_tune)),
        ("band_lo", curva(ew, w_band_lo)),
        ("band_hi", curva(ew, w_band_hi)),
        ("h1_2010", d_lin),
    ]
    escreve(os.path.join(a.outdir, "tabela_W.dat"),
            "Painel direito: sigma(gamma p -> J/psi X) [nb]", "W_gp [GeV]", t2)

    # ---------------- inventario ----------------
    print("Inventario dos elementos gerados")
    for arq, blocos in (("tabela_PT.dat", t1), ("tabela_W.dat", t2)):
        print(f"  {arq}:")
        for nome, linhas in blocos:
            print(f"    {nome:<9s} {len(linhas):2d} linhas")


if __name__ == "__main__":
    main()
