#include "glm/fwd.hpp"
#include "spdlog/spdlog.h"
#include <Utils.hpp>

namespace EGEOM {

int bSplineFindSpan(int n, int p, float u, std::vector<float> U) {
  if (u == U[n + 1]) {
    return n;
  }
  int low = p;
  int high = n + 1;
  int mid = (low + high) / 2;
  while (u < U[mid] || u >= U[mid + 1]) {
    if (u < U[mid])
      high = mid;
    else
      low = mid;
    mid = (low + high) / 2;
  }
  return mid;
}

int binomialCoeff(int n, int k) {
  // Base Cases
  if (k > n)
    return 0;
  if (k == 0 || k == n)
    return 1;

  // Recur
  return binomialCoeff(n - 1, k - 1) + binomialCoeff(n - 1, k);
}

std::vector<float> bSplineBasisFunc(int i, float u, int p,
                                    std::vector<float> U) {
  std::vector<float> N(p + 1, 0);
  std::vector<float> left(p + 1, 0);
  std::vector<float> right(p + 1, 0);

  N[0] = 1.0f;
  for (int j = 1; j <= p; j++) {
    left[j] = u - U[i + 1 - j];
    right[j] = U[i + j] - u;
    float saved = 0.0f;
    for (int r = 0; r < j; r++) {
      float temp = N[r] / (right[r + 1] + left[j - r]);
      N[r] = saved + right[r + 1] * temp;
      saved = left[j - r] * temp;
    }
    N[j] = saved;
  }
  return N;
}

Matrix<float> dersBasisFunc(int i, float u, int p, int n,
                            std::vector<float> U) {
  Matrix<float> ders(n + 1, p + 1);
  Matrix<float> ndu(p + 1, p + 1);
  std::vector<float> left(p + 1, 0);
  std::vector<float> right(p + 1, 0);
  Matrix<float> a(2, p + 1);
  ndu[0][0] = 1.0f;
  for (auto j = 1; j <= p; j++) {
    left[j] = u - U[i + 1 - j];
    right[j] = U[i + j] - u;
    float saved = 0.0f;
    for (auto r = 0; r < j; r++) {
      ndu[j][r] = right[r + 1] + left[j - r];
      float temp = ndu[r][j - 1] / ndu[j][r];
      ndu[r][j] = saved + right[r + 1] * temp;
      saved = left[j - r] * temp;
    }
    ndu[j][j] = saved;
  }
  for (auto j = 0; j <= p; j++) {
    ders[0][j] = ndu[j][p];
  }
  for (auto r = 0; r <= p; r++) {
    int s1 = 0;
    int s2 = 1;
    a[0][0] = 1.0f;
    for (auto k = 1; k <= n; k++) {
      float d = 0.0f;
      int rk = r - k;
      int pk = p - k;
      if (r >= k) {
        a[s2][0] = a[s1][0] / ndu[pk + 1][rk];
        d = a[s2][0] * ndu[rk][pk];
      }
      int j1 = rk >= -1 ? 1 : -rk;
      int j2 = r - 1 <= pk ? k - 1 : p - r;
      for (auto j = j1; j <= j2; j++) {
        a[s2][j] = (a[s1][j] - a[s1][j - 1]) / ndu[pk + 1][rk + j];
        d += a[s2][j] * ndu[rk + j][pk];
      }
      if (r <= pk) {
        a[s2][k] = -a[s1][k - 1] / ndu[pk + 1][r];
        d += a[s2][k] * ndu[r][pk];
      }
      ders[k][r] = d;
      int j = s1;
      s1 = s2;
      s2 = j;
    }
  }
  int r = p;
  for (auto k = 1; k <= n; k++) {
    for (auto j = 0; j < p; j++) {
      ders[k][j] *= r;
    }
    r *= p - k;
  }
  return ders;
}
} // namespace EGEOM
