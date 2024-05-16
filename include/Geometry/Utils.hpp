#pragma once
#include <assert.h>
#include <vector>

namespace EGEOM {
int bSplineFindSpan(int n, int p, float u, std::vector<float> U);
std::vector<float> bSplineBasisFunc(int i, float u, int p,
                                    std::vector<float> U);

template <typename T> class Matrix {
  class MatrixRow {
    T *data_ = nullptr;
    int size_;

  public:
    MatrixRow(int n) : size_(n) { data_ = new T[size_](); }

    MatrixRow(const std::initializer_list<T> &il)
        : size_(il.size()), data_(new T[size_]) {
      auto i = 0;
      for (T el : il) {
        data_[i] = el;
        i++;
      }
    }

    MatrixRow() : size_(0) {}

    ~MatrixRow() { delete[] data_; }

    T &operator[](int i) {
      assert(i < size_);
      return data_[i];
    }
  };

  MatrixRow *rows_;
  int n_;
  int m_;

public:
  Matrix(int n, int m) : n_(n), m_(m), rows_(new MatrixRow[n]) {
    for (auto i = 0; i < n_; ++i) {
      rows_[i] = *(new MatrixRow(m));
    }
  }

  Matrix(const std::initializer_list<std::initializer_list<T>> &il)
      : n_(il.size()) {
    rows_ = new MatrixRow[il.size()]{};
    auto i = 0;
    for (auto row : il) {
      rows_[i] = *(new MatrixRow(row));
      i++;
    }
  }

  ~Matrix() { delete[] rows_; }

  MatrixRow &operator[](int i) {
    assert(i < n_);
    return rows_[i];
  }
};

int binomialCoeff(int n, int k);

Matrix<float> dersBasisFunc(int i, float u, int p, int n, std::vector<float> U);
} // namespace EGEOM
