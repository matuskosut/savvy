/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "savvy/reader.hpp"

namespace savvy
{
  //################################################################//
  const std::vector<std::string> reader_base::empty_string_vector;
  const std::vector<std::pair<std::string, std::string>> reader_base::empty_string_pair_vector;

  const std::vector<std::string>& reader_base::info_fields() const
  {
    if (sav_impl())
      return sav_impl()->info_fields();
    else if (vcf_impl())
      return vcf_impl()->info_fields();
    return empty_string_vector;
  }

  const std::vector<std::string>& reader_base::samples() const
  {
    reader_base::sample_iterator ret;
    if (sav_impl())
      return sav_impl()->samples();
    else if (vcf_impl())
      return vcf_impl()->samples();
    return empty_string_vector;
  }

  const std::vector<std::pair<std::string,std::string>>& reader_base::headers() const
  {
    reader_base::sample_iterator ret;
    if (sav_impl())
      return sav_impl()->headers();
    else if (vcf_impl())
      return vcf_impl()->headers();
    return empty_string_pair_vector;
  }

  std::vector<std::string> reader_base::subset_samples(const std::set<std::string>& subset)
  {
    if (sav_impl())
      return sav_impl()->subset_samples(subset);
    else if (vcf_impl())
      return vcf_impl()->subset_samples(subset);
    return std::vector<std::string>();
  }
  //################################################################//

  //################################################################//
  std::vector<std::string> indexed_reader::chromosomes() const
  {
    if (sav_reader_)
      return sav_reader_->chromosomes();
    else if (vcf_reader_)
      return vcf_reader_->chromosomes();
    return {};
  }

  void indexed_reader::reset_region(const region& reg)
  {
    if (sav_reader_)
      sav_reader_->reset_region(reg);
    else if (vcf_reader_)
      vcf_reader_->reset_region(reg);
  }
  //################################################################//
}