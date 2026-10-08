// Macro multi-painel dirigida por rootMetaData.txt
//
// Uso: root -l -b -q 'gamma2T4c_ROOTsetup.C'
//      root -l -b -q 'gamma2T4c_ROOTsetup.C("meta.txt", "saida.tex")'
//
// ---------------------------------------------------------------------------
// FORMATO DO METADATA (linhas com '#' no início são comentários)
//
//   painel:                      novo painel (opcional se houver só um)
//   eixoX: / eixoY:              títulos dos eixos (LaTeX do ROOT)
//   xlim: a b / ylim: a b        range (opcional; senão vem dos dados)
//   logy: 0|1                    escala log em Y (padrão: 1)
//   ndivx: 405 / ndivy: 808      divisões dos eixos (padrão 510 / 808)
//   offy: 1.1                    offset do título do eixo Y
//   linha0: 0|1                  linha tracejada em x=0 (padrão: 1)
//   aux: texto                   texto cinza no canto superior esquerdo
//   titulo: texto                título interno
//   texto: x y [tam=] [cor=] [alinha=] | conteúdo     (x,y em NDC do painel)
//   texto: + [tam=] | conteúdo   linha abaixo da anterior (passo: ver abaixo)
//   passo: 0.037                 espaçamento do "texto: +" seguintes
//   legenda: x1 y1 x2 y2 [tam=] [moldura=1] | cabeçalho
//   legtxt: texto                entrada de legenda sem marcador
//   gap: xmin xmax [cor=] [estilo=] [alpha=] | rótulo
//   banda: i j [cor=] [estilo=] [alpha=] | rótulo   (i,j = nº das curvas, 1-based)
//   curvas:                      a partir daqui, uma curva por linha:
//   arquivo [tipo=] [serie=] [cor=] [estilo=] [larg=] [marc=] [tam=] [oculta=1] | legenda
//
// tipo: line (x y) | step (xlo xhi y) | points (xlo xhi y eyl eyh, ou x y)
// serie=nome: usa só as linhas do arquivo que começam com "nome"
// cor: #rrggbb | número | nome ROOT (kRed, kBlue-9, kGray+2, ...)
// oculta=1: a curva não é desenhada nem entra na legenda (serve p/ banda)
// Separador de legenda: " | " (com espaços). Sem ele, o resto da linha é a legenda
// (formato antigo: "arquivo legenda").
// ---------------------------------------------------------------------------

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <cctype>
#include <cstdlib>
#include <algorithm>
#include <utility>

// ============================ ESTRUTURAS ============================
struct Cur   { std::string arq, tipo = "line", serie, leg, cor;
               int estilo = 1, marc = 20, larg = -1; double tam = 1.0; bool oculta = false; };
struct Banda { int a = 1, b = 2, estilo = 1001; double alpha = 1.0;
               std::string cor = "kGray+1", leg; };
struct Gap   { double xmin = 0, xmax = 0, alpha = 0.75; int estilo = 3004;
               std::string cor = "kBlue-9", leg; };
struct Txt   { double x = 0.2, y = 0.9, tam = 0.034; int alinha = 12;
               std::string cor, txt; };
struct Painel {
    std::string eixoX, eixoY, aux, titulo;
    double xlim[2] = {0, 0}, ylim[2] = {0, 0};
    bool temXlim = false, temYlim = false, linha0 = true, usado = false;
    int logy = -1, ndivx = 510, ndivy = 808;
    double offy = -1, passo = 0.037;
    bool temLeg = false;
    double lx1 = 0.70, ly1 = 0.45, lx2 = 0.80, ly2 = 0.85, ltam = -1;
    int lmold = 0;
    std::string lcab;
    std::vector<Cur> curvas;
    std::vector<Banda> bandas;
    std::vector<Gap> gaps;
    std::vector<std::string> ltxt;
    std::vector<Txt> textos;
    std::vector<std::pair<char,int>> ordem;   // ordem das entradas de legenda
};
struct Lim {
    double xmin = 1e300, xmax = -1e300, ymin = 1e300, ymax = -1e300;
    void Add(double x, double y) {
        xmin = std::min(xmin, x); xmax = std::max(xmax, x);
        ymin = std::min(ymin, y); ymax = std::max(ymax, y);
    }
};

// ============================ UTILITÁRIOS ============================
static std::string Trim(std::string s) {
    const char* ws = " \t\r\n";
    s.erase(0, s.find_first_not_of(ws));
    s.erase(s.find_last_not_of(ws) + 1);
    return s;
}

static bool Chave(const std::string& l, const char* k, std::string& resto) {
    std::string kk(k);
    if (l.rfind(kk, 0) != 0) return false;
    resto = Trim(l.substr(kk.size()));
    return true;
}

// separa "esq | dir" (somente " | " com espaços)
static void Divide(const std::string& s, std::string& esq, std::string& dir, bool& tem) {
    size_t p = s.find(" | ");
    if (p == std::string::npos) { esq = s; dir.clear(); tem = false; }
    else { esq = Trim(s.substr(0, p)); dir = Trim(s.substr(p + 3)); tem = true; }
}

typedef std::map<std::string, std::string> KV;

static void Tokens(const std::string& s, std::vector<std::string>& pos, KV& kv) {
    std::istringstream iss(s);
    std::string t;
    while (iss >> t) {
        size_t e = t.find('=');
        if (e != std::string::npos && e > 0) kv[t.substr(0, e)] = t.substr(e + 1);
        else pos.push_back(t);
    }
}

static double Num(const KV& kv, const char* k, double def) {
    auto it = kv.find(k);
    return it == kv.end() ? def : std::atof(it->second.c_str());
}
static std::string Str(const KV& kv, const char* k, const std::string& def) {
    auto it = kv.find(k);
    return it == kv.end() ? def : it->second;
}

// "#rrggbb" | inteiro | nome ROOT com offset opcional (kBlue-9, gray+2, ...)
static int ParseCor(const std::string& s, int def) {
    if (s.empty()) return def;
    if (s[0] == '#') return TColor::GetColor(s.c_str());
    if (std::isdigit((unsigned char)s[0])) return std::atoi(s.c_str());
    size_t p = s.find_first_of("+-", 1);
    std::string nome = s.substr(0, p);
    int off = (p == std::string::npos) ? 0 : std::atoi(s.c_str() + p);
    for (auto& ch : nome) ch = (char)std::tolower((unsigned char)ch);
    if (!nome.empty() && nome[0] == 'k') nome.erase(0, 1);
    static const std::map<std::string, int> m = {
        {"white", kWhite}, {"black", kBlack}, {"gray", kGray}, {"grey", kGray},
        {"red", kRed}, {"green", kGreen}, {"blue", kBlue}, {"yellow", kYellow},
        {"magenta", kMagenta}, {"cyan", kCyan}, {"orange", kOrange},
        {"spring", kSpring}, {"teal", kTeal}, {"azure", kAzure},
        {"violet", kViolet}, {"pink", kPink}
    };
    auto it = m.find(nome);
    if (it == m.end()) {
        std::cerr << "Cor desconhecida: " << s << " (usando padrão)\n";
        return def;
    }
    return it->second + off;
}

// ============================ LEITURA DO METADATA ============================
static bool LeMeta(const char* arq, std::vector<Painel>& P) {
    std::ifstream meta(arq);
    if (!meta.is_open()) {
        std::cerr << "Erro ao abrir " << arq << "\n";
        return false;
    }
    auto cur = [&]() -> Painel& {
        if (P.empty()) P.emplace_back();
        P.back().usado = true;
        return P.back();
    };

    static const char* chavesCurva[] = {"tipo","cor","estilo","larg","marc","tam","serie","oculta"};
    std::string linha, r;
    bool lendoCurvas = false;

    while (std::getline(meta, linha)) {
        linha = Trim(linha);
        if (linha.empty() || linha[0] == '#') continue;

        if (Chave(linha, "painel:", r)) {
            if (P.empty() || P.back().usado) P.emplace_back();
            lendoCurvas = false;
        }
        else if (Chave(linha, "eixoX:",  r)) cur().eixoX  = r;
        else if (Chave(linha, "eixoY:",  r)) cur().eixoY  = r;
        else if (Chave(linha, "aux:",    r)) cur().aux    = r;
        else if (Chave(linha, "titulo:", r)) cur().titulo = r;
        else if (Chave(linha, "logy:",   r)) cur().logy   = std::atoi(r.c_str());
        else if (Chave(linha, "ndivx:",  r)) cur().ndivx  = std::atoi(r.c_str());
        else if (Chave(linha, "ndivy:",  r)) cur().ndivy  = std::atoi(r.c_str());
        else if (Chave(linha, "offy:",   r)) cur().offy   = std::atof(r.c_str());
        else if (Chave(linha, "passo:",  r)) cur().passo  = std::atof(r.c_str());
        else if (Chave(linha, "linha0:", r)) cur().linha0 = (std::atoi(r.c_str()) != 0);
        else if (Chave(linha, "xlim:", r)) {
            Painel& p = cur();
            std::istringstream iss(r);
            p.temXlim = static_cast<bool>(iss >> p.xlim[0] >> p.xlim[1]);
        }
        else if (Chave(linha, "ylim:", r)) {
            Painel& p = cur();
            std::istringstream iss(r);
            p.temYlim = static_cast<bool>(iss >> p.ylim[0] >> p.ylim[1]);
        }
        else if (Chave(linha, "texto:", r)) {
            Painel& p = cur();
            std::string esq, dir; bool tb;
            Divide(r, esq, dir, tb);
            if (!tb || dir.empty()) {
                std::cerr << "texto: faltando ' | conteúdo' em: " << linha << "\n";
                continue;
            }
            std::vector<std::string> pos; KV kv;
            Tokens(esq, pos, kv);
            Txt t; t.txt = dir;
            if (!pos.empty() && pos[0] == "+") {
                if (!p.textos.empty()) {
                    const Txt& a = p.textos.back();
                    t.x = a.x; t.y = a.y - p.passo;
                    t.tam = a.tam; t.alinha = a.alinha; t.cor = a.cor;
                }
            } else if (pos.size() >= 2) {
                t.x = std::atof(pos[0].c_str());
                t.y = std::atof(pos[1].c_str());
            }
            t.tam    = Num(kv, "tam", t.tam);
            t.alinha = (int)Num(kv, "alinha", t.alinha);
            t.cor    = Str(kv, "cor", t.cor);
            p.textos.push_back(t);
        }
        else if (Chave(linha, "legenda:", r)) {
            Painel& p = cur();
            std::string esq, dir; bool tb;
            Divide(r, esq, dir, tb);
            std::vector<std::string> pos; KV kv;
            Tokens(esq, pos, kv);
            p.temLeg = true;
            if (pos.size() >= 4) {
                p.lx1 = std::atof(pos[0].c_str()); p.ly1 = std::atof(pos[1].c_str());
                p.lx2 = std::atof(pos[2].c_str()); p.ly2 = std::atof(pos[3].c_str());
            }
            p.ltam  = Num(kv, "tam", -1);
            p.lmold = (int)Num(kv, "moldura", 0);
            p.lcab  = dir;
        }
        else if (Chave(linha, "legtxt:", r)) {
            Painel& p = cur();
            p.ltxt.push_back(r);
            p.ordem.push_back({'t', (int)p.ltxt.size() - 1});
        }
        else if (Chave(linha, "gap:", r)) {
            Painel& p = cur();
            std::string esq, dir; bool tb;
            Divide(r, esq, dir, tb);
            std::vector<std::string> pos; KV kv;
            Tokens(esq, pos, kv);
            Gap g;
            if (pos.size() >= 2) { g.xmin = std::atof(pos[0].c_str()); g.xmax = std::atof(pos[1].c_str()); }
            g.cor    = Str(kv, "cor", g.cor);
            g.estilo = (int)Num(kv, "estilo", g.estilo);
            g.alpha  = Num(kv, "alpha", g.alpha);
            g.leg    = dir;
            p.gaps.push_back(g);
            p.ordem.push_back({'g', (int)p.gaps.size() - 1});
        }
        else if (Chave(linha, "banda:", r)) {
            Painel& p = cur();
            std::string esq, dir; bool tb;
            Divide(r, esq, dir, tb);
            std::vector<std::string> pos; KV kv;
            Tokens(esq, pos, kv);
            Banda b;
            if (pos.size() >= 2) { b.a = std::atoi(pos[0].c_str()); b.b = std::atoi(pos[1].c_str()); }
            b.cor    = Str(kv, "cor", b.cor);
            b.estilo = (int)Num(kv, "estilo", b.estilo);
            b.alpha  = Num(kv, "alpha", b.alpha);
            b.leg    = dir;
            p.bandas.push_back(b);
            p.ordem.push_back({'b', (int)p.bandas.size() - 1});
        }
        else if (Chave(linha, "curvas:", r)) {
            cur();
            lendoCurvas = true;
        }
        else if (lendoCurvas) {
            Painel& p = cur();
            std::string esq, dir; bool tb;
            Divide(linha, esq, dir, tb);
            std::istringstream iss(esq);
            Cur c;
            iss >> c.arq;
            if (c.arq.empty()) continue;

            KV kv;
            std::string t, resto;
            bool opcoes = true;
            while (iss >> t) {
                bool ehOpt = false;
                size_t e = t.find('=');
                if (opcoes && e != std::string::npos && e > 0) {
                    std::string k = t.substr(0, e);
                    for (const char* ck : chavesCurva) if (k == ck) ehOpt = true;
                    if (ehOpt) kv[k] = t.substr(e + 1);
                }
                if (!ehOpt) {
                    opcoes = false;
                    resto += (resto.empty() ? "" : " ") + t;
                }
            }
            c.leg    = tb ? dir : resto;
            c.tipo   = Str(kv, "tipo", "line");
            if (c.tipo != "line" && c.tipo != "step" && c.tipo != "points") {
                std::cerr << "tipo desconhecido '" << c.tipo << "' (usando line)\n";
                c.tipo = "line";
            }
            c.serie  = Str(kv, "serie", "");
            c.cor    = Str(kv, "cor", "");
            c.estilo = (int)Num(kv, "estilo", 1);
            c.larg   = (int)Num(kv, "larg", -1);
            c.marc   = (int)Num(kv, "marc", 20);
            c.tam    = Num(kv, "tam", 1.0);
            c.oculta = (Num(kv, "oculta", 0) != 0);
            p.curvas.push_back(c);
            p.ordem.push_back({'c', (int)p.curvas.size() - 1});
        }
    }
    return true;
}

// ============================ LEITURA DOS DADOS ============================
static std::vector<std::vector<double>> LeLinhas(const std::string& nome,
                                                  const std::string& serie) {
    std::vector<std::vector<double>> rows;
    std::ifstream f(nome);
    if (!f.is_open()) {
        std::cerr << "Erro ao abrir " << nome << "\n";
        return rows;
    }
    std::string l;
    while (std::getline(f, l)) {
        l = Trim(l);
        if (l.empty() || l[0] == '#') continue;
        std::istringstream iss(l);
        if (!serie.empty()) {
            std::string nm;
            iss >> nm;
            if (nm != serie) continue;
        }
        std::vector<double> v;
        double d;
        while (iss >> d) v.push_back(d);
        if (!v.empty()) rows.push_back(v);
    }
    return rows;
}

// Cria o gráfico da curva e atualiza os limites dos dados.
static TGraph* CriaGrafico(const Cur& c, int cor, bool logY, Lim& lim) {
    auto rows = LeLinhas(c.arq, c.serie);
    std::vector<double> x, y, ex, eyl, eyh;

    for (const auto& v : rows) {
        double xa, xb, yy, el = 0, eh = 0;
        if (c.tipo == "line") {
            if (v.size() < 2) continue;
            xa = xb = v[0]; yy = v[1];
        } else if (c.tipo == "step") {
            if (v.size() < 3) continue;
            xa = v[0]; xb = v[1]; yy = v[2];
        } else {  // points
            if (v.size() >= 5)      { xa = v[0]; xb = v[1]; yy = v[2]; el = v[3]; eh = v[4]; }
            else if (v.size() >= 2) { xa = xb = v[0]; yy = v[1]; }
            else continue;
        }
        if (logY && yy <= 0) continue;

        if (c.tipo == "step") {
            x.push_back(xa); y.push_back(yy);
            x.push_back(xb); y.push_back(yy);
            lim.Add(xa, yy); lim.Add(xb, yy);
        } else if (c.tipo == "line") {
            x.push_back(xa); y.push_back(yy);
            lim.Add(xa, yy);
        } else {
            x.push_back(0.5 * (xa + xb));
            ex.push_back(0.5 * (xb - xa));
            y.push_back(yy); eyl.push_back(el); eyh.push_back(eh);
            lim.Add(xa, yy); lim.Add(xb, yy);
            if (!logY || yy - el > 0) lim.Add(xa, yy - el);
            lim.Add(xa, yy + eh);
        }
    }
    if (x.empty()) {
        std::cerr << "Sem dados válidos em " << c.arq
                  << (c.serie.empty() ? "" : " (serie " + c.serie + ")") << "\n";
        return nullptr;
    }

    TGraph* g;
    if (c.tipo == "points") {
        g = new TGraphAsymmErrors(x.size(), x.data(), y.data(), ex.data(), ex.data(),
                                  eyl.data(), eyh.data());
        g->SetMarkerStyle(c.marc);
        g->SetMarkerSize(c.tam);
        g->SetMarkerColor(cor);
    } else {
        g = new TGraph(x.size(), x.data(), y.data());
    }
    g->SetLineColor(cor);
    g->SetLineStyle(c.estilo);
    g->SetLineWidth(c.larg > 0 ? c.larg : (c.tipo == "points" ? 1 : 2));
    return g;
}

// Polígono entre duas curvas (recortado ao range visível).
static TGraph* CriaBanda(TGraph* A, bool stepA, TGraph* B, bool stepB,
                         double xmin, double xmax, double ymin, double ymax) {
    auto cy = [&](double v) { return std::min(std::max(v, ymin), ymax); };
    auto cx = [&](double v) { return std::min(std::max(v, xmin), xmax); };
    std::vector<double> bx, by;

    if (stepA && stepB) {
        // degraus: ida por A, volta pelos pontos de B
        for (int i = 0; i < A->GetN(); ++i) {
            bx.push_back(cx(A->GetX()[i])); by.push_back(cy(A->GetY()[i]));
        }
        for (int i = B->GetN() - 1; i >= 0; --i) {
            bx.push_back(cx(B->GetX()[i])); by.push_back(cy(B->GetY()[i]));
        }
    } else {
        // curvas suaves: B interpolada nos x de A
        if (A->GetN() < 2 || B->GetN() < 2) return nullptr;
        double x0 = std::max({A->GetX()[0], B->GetX()[0], xmin});
        double x1 = std::min({A->GetX()[A->GetN()-1], B->GetX()[B->GetN()-1], xmax});
        for (int i = 0; i < A->GetN(); ++i) {
            double xi = A->GetX()[i];
            if (xi < x0 || xi > x1) continue;
            bx.push_back(xi); by.push_back(cy(A->GetY()[i]));
        }
        for (int i = A->GetN() - 1; i >= 0; --i) {
            double xi = A->GetX()[i];
            if (xi < x0 || xi > x1) continue;
            bx.push_back(xi); by.push_back(cy(B->Eval(xi)));
        }
    }
    if (bx.size() < 3) return nullptr;
    return new TGraph(bx.size(), bx.data(), by.data());
}

// ============================ ESTILO ============================
static void EstiloLegenda(TLegend* l, bool moldura, double tam, double margem) {
    l->SetTextFont(42);
    l->SetTextSize(tam);
    l->SetMargin(margem);
    if (moldura) {
        l->SetBorderSize(1);
        l->SetLineColor(TColor::GetColor("#cccccc"));
        l->SetFillColor(kWhite);
        l->SetFillStyle(1001);
    } else {
        l->SetBorderSize(0);
        l->SetFillStyle(0);
    }
}

static void ConfiguraPad(TPad* p, double L, double R, double B, double T, bool logY) {
    p->SetMargin(L, R, B, T);
    if (logY) p->SetLogy();
    p->SetTicks(1, 1);
    p->SetFillStyle(4000);        // transparente: os pads se sobrepõem
    p->SetFrameFillStyle(0);
    p->SetFrameLineWidth(1);
}

static TH1* Moldura(double x0, double y0, double x1, double y1, const Painel& P,
                    bool unico, double offY) {
    TH1* f = gPad->DrawFrame(x0, y0, x1, y1);
    f->GetXaxis()->SetTitle(P.eixoX.c_str());
    f->GetYaxis()->SetTitle(P.eixoY.c_str());
    for (TAxis* ax : {f->GetXaxis(), f->GetYaxis()}) {
        ax->SetLabelFont(42);
        ax->SetTitleFont(42);
        ax->SetLabelSize(unico ? 0.045 : 0.036);
        ax->SetTitleSize(unico ? 0.060 : 0.050);
    }
    f->GetXaxis()->SetNdivisions(P.ndivx, kTRUE);
    f->GetYaxis()->SetNdivisions(P.ndivy, kTRUE);
    f->GetXaxis()->SetTitleOffset(unico ? 1.0 : 0.95);
    f->GetXaxis()->SetLabelOffset(unico ? 0.010 : 0.012);
    f->GetYaxis()->SetTitleOffset(offY);
    f->GetYaxis()->SetLabelOffset(unico ? 0.010 : 0.008);
    if (!unico) {
        f->GetXaxis()->SetTickLength(0.020);
        f->GetYaxis()->SetTickLength(0.015);
    }
    return f;
}

// ============================ MACRO PRINCIPAL ============================
void gamma2T4c_ROOTsetup(const char* metaFile = "rootMetaData.txt",
                         const char* outFile  = "../plots/tex_compile/stdRoot_out.tex") {
    const bool kLogYPadrao = true;

    gROOT->SetBatch(kTRUE);
    gStyle->SetOptStat(0);
    gStyle->SetTextFont(42);
    gStyle->SetEndErrorSize(0);   // sem "tampinhas" nas barras de erro

    // ========= METADATA =========
    std::vector<Painel> paineis;
    if (!LeMeta(metaFile, paineis)) return;
    const int N = paineis.size();
    size_t nCurvasTotal = 0;
    for (const auto& p : paineis) nCurvasTotal += p.curvas.size();
    if (N == 0 || nCurvasTotal == 0) {
        std::cerr << "Nenhuma curva definida em " << metaFile << "\n";
        return;
    }
    const bool unico = (N == 1);

    // ========= GEOMETRIA (pixels) =========
    double Wpx, Hpx, axL, axW, axGap, axT, axB;
    const double padOvL = 95, padOvR = 19;     // sobreposição entre pads (modelo)
    if (unico) {
        Wpx = 800; Hpx = 600;
        axL = 100; axW = 660; axGap = 0; axT = 35; axB = 520;
    } else {
        axL = 80; axW = 411; axGap = 70; axT = 25; axB = 329; Hpx = 376;
        Wpx = axL + N * axW + (N - 1) * axGap + 49;
    }

    const int colors[] = {
        kRed, kBlue, kGreen+2, kMagenta, kOrange+7, kCyan+2, kBlack,
        kViolet+1, kAzure+2, kSpring+5, kTeal+3, kPink+6, kGray+2
    };
    const int nColors = sizeof(colors) / sizeof(int);

    // ========= CANVAS E PADS =========
    auto* c1 = new TCanvas("c1", "Plotagem flexível", (int)Wpx, (int)Hpx);
    c1->SetFillColor(kWhite);

    std::vector<TPad*> pads(N, nullptr);
    std::vector<bool> logYs(N);
    for (int i = 0; i < N; ++i) {
        logYs[i] = (paineis[i].logy < 0) ? kLogYPadrao : (paineis[i].logy != 0);
        double aL = axL + i * (axW + axGap), aR = aL + axW;
        double px0 = (i == 0)     ? 0.0  : aL - padOvL;
        double px1 = (i == N - 1) ? Wpx  : aR + padOvR;
        double w = px1 - px0;
        pads[i] = new TPad(Form("p%d", i + 1), "", px0 / Wpx, 0.0, px1 / Wpx, 1.0);
        ConfiguraPad(pads[i], (aL - px0) / w, (px1 - aR) / w,
                     (Hpx - axB) / Hpx, axT / Hpx, logYs[i]);
        pads[i]->Draw();
    }

    // ========= PAINÉIS =========
    for (int i = 0; i < N; ++i) {
        Painel& P = paineis[i];
        const bool logY = logYs[i];

        // ---- gráficos e limites dos dados ----
        Lim lim;
        std::vector<TGraph*> gr(P.curvas.size(), nullptr);
        for (size_t k = 0; k < P.curvas.size(); ++k) {
            int cor = ParseCor(P.curvas[k].cor, colors[k % nColors]);
            gr[k] = CriaGrafico(P.curvas[k], cor, logY, lim);
        }
        if (lim.xmin > lim.xmax) {
            std::cerr << "Painel " << i + 1 << " sem dados válidos.\n";
            return;
        }

        // ---- range dos eixos ----
        double xmin = P.temXlim ? P.xlim[0] : lim.xmin;
        double xmax = P.temXlim ? P.xlim[1] : lim.xmax;
        double ymin, ymax;
        if (P.temYlim) {
            ymin = P.ylim[0]; ymax = P.ylim[1];
        } else if (logY) {
            ymin = std::pow(10, std::floor(std::log10(lim.ymin)));
            ymax = std::pow(10, std::ceil (std::log10(lim.ymax)));
        } else {
            double m = 0.05 * (lim.ymax - lim.ymin);
            ymin = lim.ymin - m; ymax = lim.ymax + m;
        }
        if (logY && ymin <= 0) {
            std::cerr << "Painel " << i + 1 << ": ylim inválido para escala log (ymin <= 0)\n";
            return;
        }

        pads[i]->cd();
        Moldura(xmin, ymin, xmax, ymax, P, unico,
                P.offy > 0 ? P.offy : (unico ? 1.0 : 1.1));

        // ---- gaps (atrás de tudo) ----
        std::vector<TObject*> gapLeg(P.gaps.size(), nullptr);
        for (size_t k = 0; k < P.gaps.size(); ++k) {
            const Gap& g = P.gaps[k];
            double lo = std::max(g.xmin, xmin), hi = std::min(g.xmax, xmax);
            if (lo >= hi) continue;   // fora do range visível
            int cor = ParseCor(g.cor, kGray);

            auto* box = new TBox(lo, ymin, hi, ymax);
            box->SetFillColorAlpha(cor, g.alpha);
            box->SetFillStyle(g.estilo);
            box->SetLineColor(cor);
            box->SetLineWidth(1);
            box->Draw("same");

            auto* lb = new TBox();
            lb->SetFillColorAlpha(cor, g.alpha);
            lb->SetFillStyle(g.estilo);
            lb->SetLineColor(cor);
            gapLeg[k] = lb;
        }

        // ---- bandas entre curvas ----
        std::vector<TObject*> bandaLeg(P.bandas.size(), nullptr);
        for (size_t k = 0; k < P.bandas.size(); ++k) {
            const Banda& b = P.bandas[k];
            int ia = b.a - 1, ib = b.b - 1;
            if (ia < 0 || ib < 0 || ia >= (int)gr.size() || ib >= (int)gr.size()
                || !gr[ia] || !gr[ib]) {
                std::cerr << "banda: índices de curva inválidos (" << b.a << ", " << b.b << ")\n";
                continue;
            }
            TGraph* poly = CriaBanda(gr[ia], P.curvas[ia].tipo == "step",
                                     gr[ib], P.curvas[ib].tipo == "step",
                                     xmin, xmax, ymin, ymax);
            if (!poly) continue;
            int cor = ParseCor(b.cor, kGray + 1);
            if (b.alpha < 1.0) poly->SetFillColorAlpha(cor, b.alpha);
            else               poly->SetFillColor(cor);
            poly->SetFillStyle(b.estilo);
            poly->SetLineWidth(0);
            poly->Draw("F");

            auto* lb = new TBox();
            if (b.alpha < 1.0) lb->SetFillColorAlpha(cor, b.alpha);
            else               lb->SetFillColor(cor);
            lb->SetFillStyle(b.estilo);
            lb->SetLineColor(cor);
            bandaLeg[k] = lb;
        }

        // ---- linha em x = 0 ----
        if (P.linha0 && xmin < 0 && xmax > 0) {
            auto* x0 = new TLine(0, ymin, 0, ymax);
            x0->SetLineStyle(2);
            x0->SetLineWidth(2);
            x0->SetLineColorAlpha(kGray + 2, 0.75);
            x0->Draw("same");
        }

        // ---- curvas: linhas/degraus na ordem do arquivo, pontos por cima ----
        for (size_t k = 0; k < gr.size(); ++k)
            if (gr[k] && !P.curvas[k].oculta && P.curvas[k].tipo != "points") gr[k]->Draw("L");
        for (size_t k = 0; k < gr.size(); ++k)
            if (gr[k] && !P.curvas[k].oculta && P.curvas[k].tipo == "points") gr[k]->Draw("PZ");

        // ---- legenda (ordem das entradas = ordem no arquivo) ----
        auto* leg = new TLegend(P.lx1, P.ly1, P.lx2, P.ly2);
        EstiloLegenda(leg, P.lmold != 0,
                      P.ltam > 0 ? P.ltam : (unico ? 0.04 : 0.030),
                      unico ? 0.25 : 0.17);
        if (!P.lcab.empty()) leg->SetHeader(P.lcab.c_str(), "C");
        int nEnt = 0;
        for (const auto& it : P.ordem) {
            const int k = it.second;
            switch (it.first) {
            case 'c':
                if (gr[k] && !P.curvas[k].oculta && !P.curvas[k].leg.empty()) {
                    leg->AddEntry(gr[k], P.curvas[k].leg.c_str(),
                                  P.curvas[k].tipo == "points" ? "pe" : "l");
                    ++nEnt;
                }
                break;
            case 'b':
                if (bandaLeg[k] && !P.bandas[k].leg.empty()) {
                    leg->AddEntry(bandaLeg[k], P.bandas[k].leg.c_str(), "f");
                    ++nEnt;
                }
                break;
            case 'g':
                if (gapLeg[k] && !P.gaps[k].leg.empty()) {
                    leg->AddEntry(gapLeg[k], P.gaps[k].leg.c_str(), "f");
                    ++nEnt;
                }
                break;
            case 't':
                leg->AddEntry((TObject*)nullptr, P.ltxt[k].c_str(), "");
                ++nEnt;
                break;
            }
        }
        if (nEnt > 0) leg->Draw();

        // ---- textos ----
        TLatex tx;
        tx.SetNDC();
        tx.SetTextFont(42);
        if (!P.aux.empty()) {
            tx.SetTextAlign(12);
            tx.SetTextSize(0.050);
            tx.SetTextColor(kGray + 2);
            tx.DrawLatex(0.14, 0.835, P.aux.c_str());
        }
        if (!P.titulo.empty()) {
            tx.SetTextAlign(22);
            tx.SetTextSize(0.060);
            tx.SetTextColor(kBlack);
            tx.DrawLatex(0.25, 0.75, P.titulo.c_str());
        }
        for (const auto& t : P.textos) {
            tx.SetTextAlign(t.alinha);
            tx.SetTextSize(t.tam);
            tx.SetTextColor(ParseCor(t.cor, kBlack));
            tx.DrawLatex(t.x, t.y, t.txt.c_str());
        }

        pads[i]->RedrawAxis();
    }

    // ========= SAÍDA =========
    c1->cd();
    c1->Modified();
    c1->Update();

    gSystem->mkdir(gSystem->DirName(outFile), kTRUE);
    c1->Print(outFile);
    std::cout << "Figura salva em " << outFile << "\n";
}
