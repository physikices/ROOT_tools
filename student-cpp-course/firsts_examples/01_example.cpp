#include <iostream>

using namespace std;

int main() {
  int preco, preco_com_desconto;
  cout << "Informe o preço do produto: ";
  cin >> preco;
  preco_com_desconto = preco * 0.9;
  cout << "Preco com desconto: "<< preco_com_desconto << endl;

  return 0;
}
