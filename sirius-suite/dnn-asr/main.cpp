// DNN for speech recognition decoding
// Yiping Kang, 2015
// ypkang@umich.edu

#include <assert.h>
#include <algorithm>
#include <fstream>

#include "caffe/caffe.hpp"

using caffe::Blob;
using caffe::Caffe;
using caffe::Net;
using caffe::Layer;

using namespace std;


int dnn_fwd(float* in, int in_size, float *out, int out_size, Net<float>* net)
{
  vector<Blob<float>* > in_blobs = net->input_blobs();

  vector<Blob<float>* > out_blobs;

  float loss;

  // Perform DNN forward pass
  in_blobs[0]->set_cpu_data(in);
  out_blobs = net->ForwradPrefilled(&loss);

  assert(out_size == out_blobs[0]->count() 
      && "Output size not expected.");

  memcpy(out, out_blobs[0]->cpu_data(), sizeof(float));

  return 0;
}

int dnn_init(Net<float>* net, string model_file)
{
  net->CopyTrainedLayersFrom(model_file);
  return 0;
}

int load_feature(float* in, string feature_file)
{
  // Read in features from file
  // First need to detect how many feature vectors 
  std::ifstream inFile(feature_file);
  int feat_cnt = std::count(std::istreambuf_iterator<char>(inFile),
      std::istreambuf_iterator<char>(), '\n');
  
  // Allocate memory for input feature array
  in = (float*)malloc(sizeof(float) * feat_cnt * 440); 

  // Read the feature in
  int idx = 0;
  inFile.open(feature_file, std::ifstream::in);
  std::string line;
  std::getline(inFile, line); // Get rid of the first line
  while(std::getline(inFile, line)){
    std::istringstream iss(line);
    float temp;
    while(iss >> temp){
      in[idx] = temp;
      idx++;
    }
  }
  // Everything should be in, check for sure
  assert(idx == feat_cnt * 440 && "Error: Read-in feature not correct.");

  return 0;

}

int main(int argc, char* argv)
{
  // Main function of DNN forward pass for speech recognition
  // Prase arguments
  if(argc != 3){
    printf("Usage: ./dnn-asr path_to_model path_to_input_feature\n");
    exit(1);
  }

  std::string model_file(argv[1]);
  std::string feature_file(argv[2]);
  
  printf("Decoding on input in %s with model in %s.\n", feature_file, model_file);

  // Load DNN model
  Net<float>* dnn = new Net<float>("dnn_asr");
  dnn_init(dnn, model_file);
  
  // Read in feature from file
  float* feature_input;
  int feat_cnt = load_feature(feature_input, feature_file);
  printf("%d feature vectors found in the input.\n", feat_cnt);

  // Perform dnn forward pass
  int in_size = feat_cnt * 440;
  int out_size = feat_cnt * 1706; 
  float* dnn_output = (float*)malloc(sizeof(float) * out_size);
  if(dnn_fwd(feature_input, in_size, dnn_output, out_size, dnn) != 0){
    printf("Error while DNN decoding. Abort.\n");
    exit(1);
  }

  // TODO: Result check
  printf("One forward pass is done.\n");

  return 0;
}
