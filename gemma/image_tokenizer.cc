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

class TokenizerArgs : public ArgsBase<TokenizerArgs> {
 public:
  TokenizerArgs(int argc, char* argv[]) { InitAndParse(argc, argv); }
  // TODO(atv): Add weights path, tokenizer path
  TokenizerArgs(const std::string& tokenizer_path,
                const std::string& weights_path,
                const std::string& image_file_path,
                const std::string& image_tokens_out_path) {
    Init();
    tokenizer.path = tokenizer_path;
    weights.path = weights_path;
    image_file.path = image_file_path;
    image_tokens_out.path = image_tokens_out_path;
  }

  const char* Validate() {
    if (tokenizer.path.empty()) {
        return "Missing --tokenizer flag.";
    }
    if (!tokenizer.Exists()) {
        return "Can't open file specified with --tokenizer.";
    }

    if (weights.path.empty()) {
        return "Missing --weights flag.";
    }
    if (!weights.Exists()) {
        return "Can't open file specified with --weights.";
    }

    if (image_file.path.empty()) {
        return "Missing --image_file flag.";
    }
    if (!image_file.Exists()) {
        return "Can't open file specified with --image_file.";
    }

    if (image_tokens_out.path.empty()) {
        return "Missing --image_tokens_out flag.";
    }
    return nullptr;
  }

  Path tokenizer;
  Path weights;
  Path image_file;
  Path image_tokens_out;

  template <class Visitor>
  void ForEach(const Visitor& visitor) {
    visitor(tokenizer, "tokenizer", Path(),
            "Path name of tokenizer model file.\n Required.");
    visitor(weights, "weights", Path(),
            "Path name of weights model file.\n Required.");
    visitor(image_file, "image_file", Path(),
            "Path name of image file (.ppm).\n Required.");
    visitor(image_tokens_out, "image_tokens_out", Path(),
            "Path name of token output file.\n Required.");
  }
};

void Run(TokenizerArgs& tokenizer) {
    std::mt19937 gen;
    InferenceArgs inference;
    inference.deterministic = true;
    InitGenerator(inference, gen);

    AppArgs app;
    NestedPools pools = CreatePools(app);
    Allocator::Init(pools.Topology());

    LoaderArgs loader(tokenizer.tokenizer.path, tokenizer.weights.path, "paligemma-224-vit");
    loader.Validate();

    Gemma model = CreateGemma(loader, pools);

    Image image;
    ImageTokens image_tokens;
    image_tokens = ImageTokens(Extents2D(model.GetModelConfig().vit_seq_len, model.GetModelConfig().model_dim));
    HWY_ASSERT(image.ReadPPM(tokenizer.image_file.path));
    image.Resize();
    RuntimeConfig runtime_config = {
        .gen = &gen,
        .verbosity = 1,
        .use_spinning = Tristate::kDefault,
    };
    model.GenerateImageTokens(runtime_config, image, image_tokens);

    if (!tokenizer.image_tokens_out.path.empty()) {
      auto image_tokens_out_file = gcpp::OpenFileOrNull(tokenizer.image_tokens_out, "w+");
      image_tokens_out_file->Write(image_tokens.All(), image_tokens.NumBytes(), 0);
      fprintf(stderr,
        "Wrote tokens to %s\n", tokenizer.image_tokens_out.path.c_str()
      );
    }
}

}  // namespace gcpp

int main(int argc, char** argv) {
    gcpp::TokenizerArgs tokenizer(argc, argv);

    if (const char* error = tokenizer.Validate()) {
      HWY_ABORT("\nInvalid args: %s", error);
    }

    gcpp::Run(tokenizer);

    return 0;
}