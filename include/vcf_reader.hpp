#ifndef LIBVC_VCF_READER_HPP
#define LIBVC_VCF_READER_HPP

#include "allele_status.hpp"

#include <iterator>
#include <string>
#include <vector>


//namespace vc
//{
//namespace vcf
//{
#include "vcf.h"
//}
//}

namespace vc
{
  namespace vcf
  {

    class marker
    {
    public:
      class const_iterator
      {
      public:
        typedef const_iterator self_type;
        typedef std::ptrdiff_t difference_type;
        typedef allele_status value_type;
        typedef const value_type& reference;
        typedef const value_type* pointer;
        typedef std::bidirectional_iterator_tag iterator_category;
      public:
        const_iterator(const marker& parent, std::uint64_t index) : parent_(&parent), cur_(index) {}
        self_type& operator--(){ --cur_; return *this; }
        self_type operator--(int) { self_type r = *this; --cur_; return r; }
        self_type& operator++(){ ++cur_; return *this; }
        self_type operator++(int) { self_type r = *this; ++cur_; return r; }
        reference operator*() { return (*parent_)[cur_]; }
        pointer operator->() { return &(*parent_)[cur_]; }
        bool operator==(const self_type& rhs) { return cur_ == rhs.cur_; }
        bool operator!=(const self_type& rhs) { return cur_ != rhs.cur_; }
      private:
        const marker* parent_;
        std::size_t cur_;
      };

      marker(int* gt, int num_gt, std::uint16_t allele_index);
      ~marker();

      const allele_status& operator[](std::size_t i) const;
      std::uint64_t haplotype_count() const { return static_cast<std::uint64_t>(num_gt_); }
      const_iterator begin() const { return const_iterator(*this, 0); }
      const_iterator end() const { return const_iterator(*this, haplotype_count()); }
    private:
      int* gt_;
      int num_gt_;
      const std::uint32_t allele_index_;

      static const allele_status const_is_missing;
      static const allele_status const_has_ref;
      static const allele_status const_has_alt;
    };

    class block
    {
    public:
      class const_iterator
      {
      public:
        typedef const_iterator self_type;
        typedef std::ptrdiff_t difference_type;
        typedef marker value_type;
        typedef const value_type& reference;
        typedef const value_type* pointer;
        typedef std::random_access_iterator_tag iterator_category;

        const_iterator(pointer ptr) : ptr_(ptr) { }
        self_type& operator--() { --ptr_; return *this; }
        self_type operator--(int) { self_type r = *this; --ptr_; return r; }
        self_type& operator++() { ++ptr_; return *this; }
        self_type operator++(int) { self_type r = *this; ++ptr_; return r; }
        reference operator*() { return *ptr_; }
        pointer operator->() { return ptr_; }
        bool operator==(const self_type& rhs) { return ptr_ == rhs.ptr_; }
        bool operator!=(const self_type& rhs) { return ptr_ != rhs.ptr_; }
      private:
        pointer ptr_;
      };

      block();
      ~block();

      const_iterator begin() const { return const_iterator(this->markers_.data()); }
      const_iterator end() const { return const_iterator(this->markers_.data() + this->markers_.size()); }
      const marker& operator[](std::size_t i) const;
      std::size_t marker_count() { return markers_.size(); }
      int sample_count() { return num_samples_; }

      static bool read_block(block& destination, htsFile* hts_file_, bcf_hdr_t* hts_hdr_);
    private:
      std::vector<marker> markers_;
      bcf1_t* hts_rec_;
      int num_gt_;
      int* gt_;
      int gt_sz_;
      int num_samples_;
    };

    class reader
    {
    public:
      class input_iterator
      {
      public:
        typedef input_iterator self_type;
        typedef std::ptrdiff_t difference_type;
        typedef marker value_type;
        typedef const value_type& reference;
        typedef const value_type* pointer;
        typedef std::input_iterator_tag iterator_category;
        typedef block buffer;

        input_iterator() : file_reader_(nullptr), buffer_(nullptr), i_(0) {}
        input_iterator(reader& file_reader, block& buffer) : file_reader_(&file_reader), buffer_(&buffer), i_(0)
        {
          file_reader_->read_next_block(*buffer_);
        }
        void increment()
        {
          if (i_ < buffer_->marker_count())
            ++i_;
          else
          {
            i_ = 0;
            if (!file_reader_->read_next_block(*buffer_))
              file_reader_ = nullptr;
          }
        }
        self_type& operator++(){ increment(); return *this; }
        void operator++(int) { increment(); }
        reference operator*() { return (*buffer_)[i_]; }
        pointer operator->() { return &(*buffer_)[i_]; }
        bool operator==(const self_type& rhs) { return (file_reader_ == rhs.file_reader_); }
        bool operator!=(const self_type& rhs) { return (file_reader_ != rhs.file_reader_); }
      private:
        reader* file_reader_;
        block* buffer_;
        std::uint32_t i_;
      };

      reader(const std::string& file_path);
      ~reader();
      bool read_next_block(block& destination);
    private:
      htsFile* hts_file_;
      bcf_hdr_t* hts_hdr_;
    };
  }
}

#endif //LIBVC_VCF_READER_HPP