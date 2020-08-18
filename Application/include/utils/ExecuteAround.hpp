/*
Copyright (c) 2016 Arnaud Bienner

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef UTILS_EXECUTEAROUND_HPP
#define UTILS_EXECUTEAROUND_HPP

template<typename T, template<typename> class SharedPtr, typename FuncIn, typename FuncOut>
class ExecuteAround {
private:
  class ExecuteAroundProxy {
  public:
    ExecuteAroundProxy(T* ptr, FuncIn func_in, FuncOut func_out)
    : ptr_(ptr)
    , func_out_(func_out) {
      func_in();
    }

    ~ExecuteAroundProxy() {
      func_out_();
    }

    const T* operator->() const {
      return ptr_;
    }

    T* operator->() {
      return ptr_;
    }

  private:
    T* ptr_;
    FuncIn func_in_;
    FuncOut func_out_;

  };

public:
  ExecuteAround(T* ptr, FuncIn func_in, FuncOut func_out)
  : ptr_(ptr)
  , func_in_(func_in)
  , func_out_(func_out) {}

  ExecuteAround() {

  }

  ExecuteAroundProxy operator->() const {
    return ExecuteAroundProxy(ptr_.get(), func_in_, func_out_);
  }

private:
  SharedPtr<T> ptr_;
  FuncIn func_in_;
  FuncOut func_out_;
};

#endif