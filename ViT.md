# Build and execute the ViT extractor
```sh
export PALIGEMMA_PATH=/path/to/paligemma
bazel build -c opt --cxxopt=-std=c++20 //:vit_extractor
bazel-bin/vit_extractor --weights ${PALIGEMMA_PATH}/paligemma-3b-mix-224-sfp.sbs --output ${PALIGEMMA_PATH}/vit.sbs
```

# Build and execute the tokenizer
```sh
export PALIGEMMA_PATH=/path/to/paligemma
bazel build -c opt --cxxopt=-std=c++20 //:image_tokenizer
bazel-bin/image_tokenizer --tokenizer ${PALIGEMMA_PATH}/paligemma_tokenizer.model --weights ${PALIGEMMA_PATH}/vit.sbs --image_file ${PALIGEMMA_PATH}/input.ppm --image_tokens_out ${PALIGEMMA_PATH}/input.tokens
```

# Build and run gemma with the image tokens only
```sh
export PALIGEMMA_PATH=/path/to/paligemma
bazel build -c opt --cxxopt=-std=c++20 //:gemma
bazel-bin/gemma --tokenizer ${PALIGEMMA_PATH}/paligemma_tokenizer.model --model paligemma-224 --weights ${PALIGEMMA_PATH}/paligemma-3b-mix-224-sfp.sbs --image_tokens_file ${PALIGEMMA_PATH}/input.tokens
```