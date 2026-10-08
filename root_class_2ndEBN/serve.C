void serve() {
  auto h = new TH1F("h", "Gaussiana;x;N", 100, -4, 4);
  h->FillRandom("gaus", 10000);

  auto s = new THttpServer("http:8877?loopback");
  s->Register("/", h);
}
