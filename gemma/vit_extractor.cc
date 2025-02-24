// Copyright 2025 Google LLC
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>
#include <string>

#include "evals/benchmark_helper.h"
#include "util/args.h"  // HasHelp

#if (!defined(HWY_VERSION_LT) || HWY_VERSION_LT(1, 2)) && !HWY_IDE
#error "Please update to version 1.2 of github.com/google/highway."
#endif
#if HWY_CXX_LANG < 201703L
#error "Gemma.cpp requires C++17, please pass -std=c++17."
#endif

namespace gcpp {

class VitExtractorArgs : public ArgsBase<VitExtractorArgs> {
 public:
  VitExtractorArgs(int argc, char* argv[]) { InitAndParse(argc, argv); }
  VitExtractorArgs(const std::string& weights_path, const std::string& output_path) {
    Init();
    weights.path = weights_path;
    output.path = output_path;
  }

  const char* Validate() {
    if (weights.path.empty()) {
        return "Missing --weights flag.";
    }
    if (!weights.Exists()) {
        return "Can't open file specified with --weights.";
    }
    if (output.path.empty()) {
        return "Missing --output flag.";
    }
    return nullptr;
  }

  Path weights;
  Path output;

  template <class Visitor>
  void ForEach(const Visitor& visitor) {
    visitor(weights, "weights", Path(),
            "Path name of weights model file.\n Required.");
    visitor(output, "output", Path(),
            "Path name out VIT output file.\n Required.");
  }
};

// Storage for data copied from the original weights.
std::vector<std::vector<uint8_t>> vit_data;
void CopyByKey(BlobReader& reader, BlobWriter& writer, hwy::uint128_t key, size_t bytes, hwy::ThreadPool& pool, const Path& filename) {
  auto& data = vit_data.emplace_back(bytes);
  reader.ReadOne(key, data.data(), bytes);
  writer.Add(key, data.data(), bytes);
}

void Run(VitExtractorArgs& extractor) {
    AppArgs app;
    NestedPools pools = CreatePools(app);
    Allocator::Init(pools.Topology());
    auto& pool = pools.Pool();

    ModelWeightsStorage model;
    ModelInfo info;
    info.model = Model::PALIGEMMA_224;
    info.weight = Type::kSFP;
    model.Load(extractor.weights, info.model, info.weight, pools.Pool());

    if (model.Config().vit_layer_configs.empty()) {
        HWY_ABORT("No VIT layers in this model!");
    }

    ModelConfig vit_config = VitConfig(model.Config());
    BlobReader vit_reader;
    vit_reader.Open(extractor.weights);

    BlobWriter vit_writer;

    // Iterate over the ViT layers and copy out their data.
    std::vector<LayerWeightsPtrs<SfpStream>*> vit_layers(vit_config.layer_configs.size());
    for (size_t layer = 0; layer < vit_config.layer_configs.size(); ++layer) {
        auto* layer_weights = model.GetWeightsOfType<SfpStream>()->GetVitLayer(layer);
        vit_layers[layer] = layer_weights;
        LayerWeightsPtrs<SfpStream>::ForEachTensor(vit_layers, layer, ForEachType::kIgnoreNulls,
        [&](const char* name, hwy::Span<MatPtr*> tensors) {
            auto tensor = tensors[0];
            auto key = MakeKey(name);
            CopyByKey(vit_reader, vit_writer, key, tensor->SizeBytes(), pool, extractor.output);
        });
    }

    // Extract common ViT layers from the weights.
    auto enc_norm_bias = model.GetWeightsOfType<SfpStream>()->vit_encoder_norm_bias;
    CopyByKey(vit_reader, vit_writer, MakeKey("Benc_norm_bias"), enc_norm_bias.SizeBytes(), pool, extractor.output);

    auto enc_norm_scale = model.GetWeightsOfType<SfpStream>()->vit_encoder_norm_scale;
    CopyByKey(vit_reader, vit_writer, MakeKey("Benc_norm_scale"), enc_norm_scale.SizeBytes(), pool, extractor.output);

    auto img_emb_bias = model.GetWeightsOfType<SfpStream>()->vit_img_embedding_bias;
    CopyByKey(vit_reader, vit_writer, MakeKey("Fimg_emb_bias"), img_emb_bias.SizeBytes(), pool, extractor.output);

    auto img_emb_kernel = model.GetWeightsOfType<SfpStream>()->vit_img_embedding_kernel;
    CopyByKey(vit_reader, vit_writer, MakeKey("Bimg_emb_kernel"), img_emb_kernel.SizeBytes(), pool, extractor.output);

    auto img_pos_emb = model.GetWeightsOfType<SfpStream>()->vit_img_pos_embedding;
    CopyByKey(vit_reader, vit_writer, MakeKey("Fimg_pos_emb"), img_pos_emb.SizeBytes(), pool, extractor.output);

    auto img_head_bias = model.GetWeightsOfType<SfpStream>()->vit_img_head_bias;
    CopyByKey(vit_reader, vit_writer, MakeKey("Fimg_head_bias"), img_head_bias.SizeBytes(), pool, extractor.output);

    auto img_head_kernel = model.GetWeightsOfType<SfpStream>()->vit_img_head_kernel;
    CopyByKey(vit_reader, vit_writer, MakeKey("Bimg_head_kernel"), img_head_kernel.SizeBytes(), pool, extractor.output);

    // Flush all extracted weights to the output file.
    vit_writer.WriteAll(pools.Pool(), extractor.output);
}

}  // namespace gcpp

int main(int argc, char** argv) {
    gcpp::VitExtractorArgs extractor(argc, argv);

    if (const char* error = extractor.Validate()) {
      // gcpp::ShowHelp(loader, inference, app);
      HWY_ABORT("\nInvalid args: %s", error);
    }

    gcpp::Run(extractor);

    return 0;
}