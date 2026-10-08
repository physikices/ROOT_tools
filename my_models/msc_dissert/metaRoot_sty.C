void metaRoot_sty() {
    // ========= CONFIGURAÇÃO BÁSICA =========
    gROOT->SetBatch(kTRUE);
    gStyle->SetOptStat(0);
    gStyle->SetTextFont(42);
    gStyle->SetTitleFont(42, "XYZ");
    gStyle->SetLabelFont(42, "XYZ");
    gStyle->SetTitleSize(0.05, "XYZ");
    gStyle->SetTitleOffset(0, "XYZ");
    gStyle->SetLabelSize(0.045, "XYZ");
    gStyle->SetPadTickX(1);
    gStyle->SetPadTickY(1);

    // ========= CONFIGURAÇÃO ===============
    std::ifstream meta("rootMetaData.txt");
    if (!meta.is_open()) {
        std::cerr << "Erro ao abrir meta.txt\n";
        return;
    }

    std::string linha;
    std::string tituloInterno, eixoX, eixoY;
    std::vector<std::string> arquivos;
    std::vector<std::string> legendas;

    bool lendoCurvas = false;

    while (std::getline(meta, linha)) {
        // Remove espaços à esquerda
        linha.erase(0, linha.find_first_not_of(" \t"));
        if (linha.empty()) continue;

        if (linha.substr(0, 7) == "titulo:") {
            tituloInterno = linha.substr(7);
            tituloInterno.erase(0, tituloInterno.find_first_not_of(" \t"));
        } else if (linha.substr(0, 6) == "eixoX:") {
            eixoX = linha.substr(6);
            eixoX.erase(0, eixoX.find_first_not_of(" \t"));
        } else if (linha.substr(0, 6) == "eixoY:") {
            eixoY = linha.substr(6);
            eixoY.erase(0, eixoY.find_first_not_of(" \t"));
        } else if (linha.substr(0, 7) == "curvas:") {
            lendoCurvas = true;
        } else if (lendoCurvas) {
            std::istringstream iss(linha);
            std::string nomeArquivo;
            if (!(iss >> nomeArquivo)) continue;

            std::string rotulo;
            std::getline(iss, rotulo);
            rotulo.erase(0, rotulo.find_first_not_of(" \t"));

            arquivos.push_back(nomeArquivo);
            legendas.push_back(rotulo);
        }
    }

    meta.close();

    int n = arquivos.size();
    if (n == 0) {
        std::cerr << "Nenhuma curva definida no meta.txt\n";
        return;
    }

    // Preparação do Canvas e Multigraph
    TCanvas* c1 = new TCanvas("c1", "Plotagem flexível", 500, 400);
    c1->SetGrid();
    // c1->SetLogx();
    // c1->SetLogy();

    TMultiGraph* mg = new TMultiGraph();
    TLegend* legenda = new TLegend(0.60, 0.50, 0.80, 0.70);
    legenda->SetBorderSize(0);
    legenda->SetFillColorAlpha(0,0);
    legenda->SetTextSize(0.04);

    int colors[] = {
        kRed, kBlue, kGreen+2, kMagenta, kOrange+7, kCyan+2, kBlack,
        kViolet+1, kAzure+2, kSpring+5, kTeal+3, kPink+6, kGray+2
    };
    int nColors = sizeof(colors)/sizeof(int);

    for (int i = 0; i < n; ++i) {
        std::ifstream arq(arquivos[i]);
        if (!arq.is_open()) {
            std::cerr << "Erro ao abrir " << arquivos[i] << "\n";
            continue;
        }

        std::vector<double> x, y;
        double xv, yv;
        while (arq >> xv >> yv) {
            x.push_back(xv);
            y.push_back(yv);
        }
        arq.close();

        if (x.empty()) {
            std::cerr << "Arquivo vazio ou mal formatado: " << arquivos[i] << "\n";
            continue;
        }

        TGraph* gr = new TGraph(x.size(), &x[0], &y[0]);

        gr->SetLineColor(colors[i % nColors]);
        gr->SetLineWidth(2);
        gr->SetMarkerStyle(0);
        // gr->SetMarkerStyle(i+20);
        // gr->SetMarkerColor(colors[i % nColors]);
        // gr->SetMarkerSize(1.0);

        mg->Add(gr);
        legenda->AddEntry(gr, legendas[i].c_str(), "lp");
    }

    // Desenha gráfico
    mg->SetTitle("; ;");
    mg->Draw("APL");

    mg->GetXaxis()->SetTitle(eixoX.c_str());
    mg->GetYaxis()->SetTitle(eixoY.c_str());

    mg->GetXaxis()->SetTitleSize(0.06);
    mg->GetXaxis()->SetTitleOffset(1.0);
    mg->GetXaxis()->SetLabelOffset(0.01);

    mg->GetYaxis()->SetTitleSize(0.06);
    mg->GetYaxis()->SetTitleOffset(1.0);
    mg->GetYaxis()->SetLabelOffset(0.01);

    // mg->GetXaxis()->SetLimits(.4, 70.0);
    // mg->GetYaxis()->SetRangeUser(0.000001, 1000000000.0);

    legenda->Draw();


    // ===== Linha vertical em x = 0 =====
    // double ymin = gPad->GetUymin();
    // double ymax = gPad->GetUymax();
    //
    // TLine *x0 = new TLine(0, ymin, 0, ymax);
    // x0->SetLineStyle(2);    // 2 = tracejada
    // x0->SetLineColor(kGray+2);
    // x0->SetLineWidth(2);
    // x0->Draw("same");
    //
    // ===== Linha horizontal em y = 1 =====
    // gPad->Update();
    // double xmin = gPad->GetUxmin();
    // double xmax = 70.0;//gPad->GetUxmax();
    //
    // TLine *y1 = new TLine(xmin, 1.0, xmax, 1.0);
    // y1->SetLineStyle(2);    // 2 = tracejada
    // y1->SetLineColor(kGray+2);
    // y1->SetLineWidth(2);
    // y1->Draw("same");

    // =======================================================
    // CAIXA AUXILIAR — TIPO DE COLISÃO
    // =======================================================

    TPaveText* collisionBox = new TPaveText(0.68, 0.90, 0.90, 0.95, "NDC");
    // collisionBox->AddText("#;PbPb #to T_{4c}+X");

    collisionBox->SetFillColorAlpha(0, 0); // transparente
    collisionBox->SetBorderSize(0);
    collisionBox->SetTextFont(42);
    collisionBox->SetTextAlign(12);        // esquerda + centro vertical
    collisionBox->SetTextSize(0.050);
    collisionBox->SetTextColor(kGray+2);

    collisionBox->Draw();

    // Título interno com LaTeX
    TPaveText* titulo = new TPaveText(0.62, 0.75, 0.90, 0.90, "NDC");
    titulo->AddText(tituloInterno.c_str());
    titulo->SetFillColorAlpha(kWhite,0.95);
    titulo->SetBorderSize(1);
    titulo->SetTextFont(42);
    titulo->SetTextSize(0.060);
    titulo->Draw();

    c1->SetGrid(0, 0);  // Grid só em X
    // c1->Update();

    // Força eixo log a existir para selecionar quantos ticks utilizar
    // c1->Update();

    // Mostra exatamente 6 valores no eixo Y
    mg->GetYaxis()->SetNdivisions(8);  

    // Opcional: nenhuma subdivisão
    // mg->GetYaxis()->SetNdivisions(600);

    // Atualiza
    // c1->Modified();

    c1->Update();

    // Controle dos ticks do eixo Y
    // mg->GetYaxis()->SetNdivisions(808, kTRUE);

    // Atualiza
    c1->Modified();
    c1->Update();


    c1->Print("../plots/tex_compile/stdRoot_out.tex");
}
