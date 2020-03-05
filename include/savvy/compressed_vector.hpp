/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LIBSAVVY_SPARSE_VECTOR_HPP
#define LIBSAVVY_SPARSE_VECTOR_HPP

#include <vector>
#include <algorithm>

namespace savvy
{
  template<typename T>
  class compressed_vector
  {
  public:
    typedef T value_type;
    typedef compressed_vector<T> self_type;
    static const T const_value_type;

    class iterator
    {
    public:
      typedef iterator self_type;
      typedef std::ptrdiff_t difference_type;
      typedef T value_type;
      typedef value_type& reference;
      typedef value_type* pointer;
      typedef std::input_iterator_tag iterator_category;

      iterator() : vec_(nullptr), beg_(nullptr), cur_(beg_) {}
      iterator(compressed_vector& parent, std::size_t off) :
        vec_(&parent),
        beg_(parent.values_.data()),
        cur_(parent.values_.data() + off)
      {

      }

      std::size_t offset() const
      {
        return vec_->offsets_[cur_ - beg_];
      }

      self_type operator++()
      {
        self_type ret = *this;
        ++cur_;
        return ret;
      }

      void operator++(int) { ++cur_; }
      reference operator*() { return *cur_; }
      pointer operator->() { return cur_; }
      bool operator==(const self_type& rhs) const { return (cur_ == rhs.cur_); }
      bool operator!=(const self_type& rhs) const { return (cur_ != rhs.cur_); }
    private:
      compressed_vector* vec_;
      const value_type*const beg_;
      value_type* cur_;
    };

    class const_iterator
    {
    public:
      typedef const_iterator self_type;
      typedef std::ptrdiff_t difference_type;
      typedef T value_type;
      typedef const value_type& reference;
      typedef const value_type* pointer;
      typedef std::input_iterator_tag iterator_category;

      const_iterator() : vec_(nullptr), beg_(nullptr), cur_(beg_) {}
      const_iterator(const compressed_vector& parent, std::size_t off) :
        vec_(&parent),
        beg_(parent.values_.data()),
        cur_(beg_ + off)
      {

      }

      std::size_t offset() const
      {
        return vec_->offsets_[cur_ - beg_];
      }

      self_type operator++()
      {
        self_type ret = *this;
        ++cur_;
        return ret;
      }

      void operator++(int) { ++cur_; }
      reference operator*() const { return *cur_; }
      const pointer operator->() const { return cur_; }
      bool operator==(const self_type& rhs) const { return (cur_ == rhs.cur_); }
      bool operator!=(const self_type& rhs) const { return (cur_ != rhs.cur_); }
    private:
      const compressed_vector* vec_;
      const value_type*const beg_;
      const value_type* cur_;
    };

    compressed_vector(std::size_t sz = 0)
    {
      resize(sz);
    }

    template <typename ValT, typename OffT>
    compressed_vector(std::size_t sz, std::size_t sp_sz, ValT val_it, OffT off_it)
    {
      assign(sz, sp_sz, val_it, off_it);
    }

    template <typename ValT>
    void assign(ValT val_it, ValT val_end)
    {
      size_ = val_end - val_it;
      values_.clear();
      offsets_.clear();
      values_.reserve(size_);
      offsets_.reserve(size_);
      for (auto it = val_it; it != val_end; ++it)
      {
        //typename std::iterator_traits<ValT>::value_type tmp = *it;
        if (*it)
        {
          values_.emplace_back(*it);
          offsets_.emplace_back(it - val_it);
        }
      }
    }

    template <typename ValT, typename OffT>
    void assign(ValT val_it, ValT val_end, OffT off_it, std::size_t sz)
    {
      size_ = sz;
      values_.clear();
      offsets_.clear();
      std::size_t sp_sz = val_end - val_it;
      values_.resize(sp_sz);
      offsets_.resize(sp_sz);
      std::copy_n(val_it, sp_sz, values_.begin());
      std::copy_n(off_it, sp_sz, offsets_.begin());
    }

    value_type& operator[](std::size_t pos)
    {
      if (offsets_.size() && offsets_.back() < pos)
      {
        offsets_.emplace_back(pos);
        values_.emplace_back();
        return values_.back();
      }
      else
      {
        auto it = std::lower_bound(offsets_.begin(), offsets_.end(), pos);
        if (it == offsets_.end() || *it != pos)
        {
          it = offsets_.insert(it, pos);
          return *(values_.insert(values_.begin() + std::distance(offsets_.begin(), it), value_type()));
        }
        return values_[it - offsets_.begin()];
      }
    }

    const_iterator cbegin() const  { return const_iterator(*this, 0); }
    const_iterator cend() const { return const_iterator(*this, this->values_.size()); }

    const_iterator begin() const  { return this->cbegin(); }
    const_iterator end() const { return this->cend(); }

    iterator begin() { return iterator(*this, 0); }
    iterator end() { return iterator(*this, this->values_.size()); }

    const value_type& operator[](std::size_t pos) const
    {
      auto it = std::lower_bound(offsets_.begin(), offsets_.end(), pos);
      if (it == offsets_.end() || *it != pos)
        return const_value_type;
      return values_[it - offsets_.begin()];
    }

    void resize(std::size_t sz, value_type val = value_type())
    {
      if (!sz)
      {
        offsets_.clear();
        values_.clear();
      }
      else if (sz < size_)
      {
        auto it = std::lower_bound(offsets_.begin(), offsets_.end(), sz);
        offsets_.erase(it, offsets_.end());
        values_.resize(offsets_.size());
      }
      else if (val != value_type())
      {
        values_.resize(sz, val);
        offsets_.resize(sz);
        for (std::size_t i = size_; i < sz; ++i)
          offsets_[i] = i;
      }

      size_ = sz;
    }

    void reserve(std::size_t non_zero_size_hint)
    {
      this->offsets_.reserve(non_zero_size_hint);
      this->values_.reserve(non_zero_size_hint);
    }

    void clear()
    {
      resize(0);
    }

    value_type operator*(const self_type& other) const
    {
      return dot(other, value_type());
    }

    value_type dot(const self_type& other) const
    {
      return dot(other, value_type());
    }

    template <typename AggregateT>
    AggregateT dot_slow(const self_type& other, AggregateT ret) const
    {
      if (size() < other.size())
      {
        auto beg_it = offsets_.begin();
        auto beg_jt = other.offsets_.begin();
        auto jt = beg_jt;
        for (auto it = beg_it; it != offsets_.end() && jt != other.offsets_.end(); ++it)
        {
          jt = std::lower_bound(jt, other.offsets_.end(), *it);
          if (jt != other.offsets_.end() && *jt == *it)
          {
            values_[it - beg_it] * other.values_[jt - beg_jt];
            ++jt;
          }
        }
      }
      else
      {
        auto beg_it = other.offsets_.begin();
        auto beg_jt = offsets_.begin();
        auto jt = beg_jt;
        for (auto it = beg_it; it != other.offsets_.end() && jt != offsets_.end(); ++it)
        {
          jt = std::lower_bound(jt, offsets_.end(), *it);
          if (jt != offsets_.end() && *jt == *it)
          {
            ret += other.values_[it - beg_it] * values_[jt - beg_jt];
            ++jt;
          }
        }
      }

      return ret;
    }

    template <typename AggregateT>
    AggregateT dot(const self_type& other, AggregateT ret) const
    {
      auto beg_it = offsets_.begin();
      auto beg_jt = other.offsets_.begin();
      auto it = beg_it;
      auto jt = beg_jt;
      while (it != offsets_.end() && jt != other.offsets_.end())
      {
        if ((*it) < (*jt))
          ++it;
        else if ((*jt) < (*it))
          ++jt;
        else
        {
          ret += values_[it - beg_it] * other.values_[jt - beg_jt];
          ++it;
          ++jt;
        }
      }

      return ret;
    }

    const std::size_t* const index_data() const { return offsets_.data(); }
    const value_type* const value_data() const { return values_.data(); }
    std::size_t size() const { return size_; }
    std::size_t non_zero_size() const { return values_.size(); }
  private:
    std::vector<value_type> values_;
    std::vector<std::size_t> offsets_;
    std::size_t size_;
  };

  template <typename T>
  const T compressed_vector<T>::const_value_type = T();
}

#endif //LIBSAVVY_SPARSE_VECTOR_HPP
