// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#define BOOST_TEST_MODULE Test Framework Utils RootTreeWriter
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <iomanip>
#include "Headers/DataHeader.h"
#include <fairmq/FairMQMessage.h>
#include <fairmq/FairMQTransportFactory.h>
#include "Framework/DataProcessingHeader.h"
#include "Framework/InputRecord.h"
#include "Framework/DataRef.h"
#include "Framework/DataRefUtils.h"
#include "DPLUtils/RootTreeWriter.h"
#include "DPLUtils/MakeRootTreeWriterSpec.h"
#include "../../Core/test/TestClasses.h"
#include <vector>
#include <memory>
#include <type_traits> // std::is_fundamental
#include <TClass.h>
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TSystem.h>

using namespace o2::framework;
using DataHeader = o2::header::DataHeader;

template <typename T>
struct BranchContent {
  using ref_type = T;
  const char* branchName;
  ref_type reference;
};

template <typename T>
bool checkBranch(TTree& tree, BranchContent<T>&& content)
{
  TBranch* branch = tree.GetBranch(content.branchName);
  BOOST_REQUIRE(branch != nullptr);
  T store;
  T* pointer = &store;
  // in general, pointer to pointer has to be used for setting the branch
  // address to a store object, however this does not work for fundamental
  // types, there the address to the variable has to be used in order
  // to read back the value. Why? no clue.
  if (std::is_fundamental<T>::value) {
    branch->SetAddress(&store);
  } else {
    branch->SetAddress(&pointer);
  }
  branch->GetEntry(0);
  BOOST_CHECK_MESSAGE(store == content.reference, "mismatch for branch " << content.branchName);
  return store == content.reference;
}

template <typename T, typename... Args>
bool checkBranch(TTree& tree, BranchContent<T>&& content, Args&&... args)
{
  return checkBranch(tree, std::forward<BranchContent<T>>(content)) && checkBranch(tree, std::forward<Args>(args)...);
}

template <typename... Args>
bool checkTree(const char* filename, const char* treename, Args&&... args)
{
  TFile* file = TFile::Open(filename);
  BOOST_REQUIRE(file != nullptr);
  TTree* tree = reinterpret_cast<TTree*>(file->GetObjectChecked(treename, "TTree"));
  BOOST_REQUIRE(tree != nullptr);
  return checkBranch(*tree, std::forward<Args>(args)...);
}

BOOST_AUTO_TEST_CASE(test_RootTreeWriter)
{
  std::string filename = "test_RootTreeWriter.root";
  const char* treename = "testtree";

  using Container = std::vector<o2::test::Polymorphic>;
  // setting up the writer with two branch definitions
  // first definition is for a single input and simple type written to one branch
  // second branch handles two inputs of the same data type, the mapping of the
  // input data to the target branch is taken from the sub specification
  RootTreeWriter writer(filename.c_str(), treename, // file and tree name
                        RootTreeWriter::BranchDef<int>{"input1", "intbranch"},
                        RootTreeWriter::BranchDef<Container>{
                          std::vector<std::string>({"input2", "input3"}), "containerbranch",
                          // define two target branches (this matches the input list)
                          2,
                          // the callback extracts the sub specification from the DataHeader as index
                          [](o2::framework::DataRef const& ref) {
                            auto const* dataHeader = DataRefUtils::getHeader<o2::header::DataHeader*>(ref);
                            return dataHeader->subSpecification;
                          },
                          // the branch names are simply built by adding the index
                          [](std::string base, size_t i) { return base + "_" + std::to_string(i); }},
                        RootTreeWriter::BranchDef<const char*>{"input4", "binarybranch"},
                        RootTreeWriter::BranchDef<std::vector<int>>{"input5", "intvecbranch"},
                        RootTreeWriter::BranchDef<std::vector<o2::test::TriviallyCopyable>>{"input6", "trivvecbranch"});

  BOOST_CHECK(writer.getStoreSize() == 5);

  // need to mimic a context to actually call the processing
  auto transport = FairMQTransportFactory::CreateTransportFactory("zeromq");
  std::vector<FairMQMessagePtr> store;

  auto createPlainMessage = [&transport, &store](DataHeader&& dh, auto& data) {
    dh.payloadSize = sizeof(data);
    dh.payloadSerializationMethod = o2::header::gSerializationMethodNone;
    DataProcessingHeader dph{0, 1};
    o2::header::Stack stack{dh, dph};
    FairMQMessagePtr header = transport->CreateMessage(stack.size());
    FairMQMessagePtr payload = transport->CreateMessage(sizeof(data));
    memcpy(header->GetData(), stack.data(), stack.size());
    memcpy(payload->GetData(), &data, sizeof(data));
    store.emplace_back(std::move(header));
    store.emplace_back(std::move(payload));
  };

  auto createVectorMessage = [&transport, &store](DataHeader&& dh, auto& data) {
    dh.payloadSize = data.size() * sizeof(typename std::remove_reference<decltype(data)>::type::value_type);
    dh.payloadSerializationMethod = o2::header::gSerializationMethodNone;
    DataProcessingHeader dph{0, 1};
    o2::header::Stack stack{dh, dph};
    FairMQMessagePtr header = transport->CreateMessage(stack.size());
    FairMQMessagePtr payload = transport->CreateMessage(dh.payloadSize);
    memcpy(header->GetData(), stack.data(), stack.size());
    memcpy(payload->GetData(), data.data(), dh.payloadSize);
    store.emplace_back(std::move(header));
    store.emplace_back(std::move(payload));
  };

  auto createSerializedMessage = [&transport, &store](DataHeader&& dh, auto& data) {
    FairMQMessagePtr payload = transport->CreateMessage();
    auto* cl = TClass::GetClass(typeid(decltype(data)));
    TMessageSerializer().Serialize(*payload, &data, cl);
    dh.payloadSize = payload->GetSize();
    dh.payloadSerializationMethod = o2::header::gSerializationMethodROOT;
    DataProcessingHeader dph{0, 1};
    o2::header::Stack stack{dh, dph};
    FairMQMessagePtr header = transport->CreateMessage(stack.size());
    memcpy(header->GetData(), stack.data(), stack.size());
    store.emplace_back(std::move(header));
    store.emplace_back(std::move(payload));
  };

  int a = 23;
  Container b{{0}};
  Container c{{21}};
  std::vector<int> intvec{10, 21, 42};
  std::vector<o2::test::TriviallyCopyable> trivvec{{10, 21, 42}, {1, 2, 3}};
  createPlainMessage(o2::header::DataHeader{"INT", "TST", 0}, a);
  createSerializedMessage(o2::header::DataHeader{"CONTAINER", "TST", 0}, b);
  createSerializedMessage(o2::header::DataHeader{"CONTAINER", "TST", 1}, c);
  createPlainMessage(o2::header::DataHeader{"BINARY", "TST", 0}, a);
  createVectorMessage(o2::header::DataHeader{"FDMTLVEC", "TST", 0}, intvec);
  createVectorMessage(o2::header::DataHeader{"TRIV_VEC", "TST", 0}, trivvec);

  // Note: InputRecord works on references to the schema and the message vector
  // so we can not specify the schema definition directly in the definition of
  // the InputRecord. Intrestingly enough, the compiler does not complain about
  // getting reference to temporary rvalue argument. So it might work if the
  // temporary argument is still in memory
  // FIXME: check why the compiler does not detect this
  std::vector<InputRoute> schema = {
    {InputSpec{"input1", "TST", "INT"}, 0, "input1", 0},       //
    {InputSpec{"input2", "TST", "CONTAINER"}, 1, "input2", 0}, //
    {InputSpec{"input3", "TST", "CONTAINER"}, 2, "input3", 1}, //
    {InputSpec{"input4", "TST", "BINARY"}, 3, "input4", 0},    //
    {InputSpec{"input5", "TST", "FDMTLVEC"}, 4, "input5", 0},  //
    {InputSpec{"input6", "TST", "TRIV_VEC"}, 5, "input6", 0},  //
  };

  auto getter = [&store](size_t i) -> char const* { return static_cast<char const*>(store[i]->GetData()); };

  InputRecord inputs{
    schema,
    InputSpan{getter, store.size()}};

  writer(inputs);
  writer.close();

  checkTree(filename.c_str(), treename,
            BranchContent<decltype(a)>{"intbranch", a},
            BranchContent<decltype(b)>{"containerbranch_0", b},
            BranchContent<decltype(c)>{"containerbranch_1", c},
            BranchContent<decltype(intvec)>{"intvecbranch", intvec},
            BranchContent<decltype(trivvec)>{"trivvecbranch", trivvec});
}

template <typename T>
using BranchDefinition = MakeRootTreeWriterSpec::BranchDefinition<T>;

BOOST_AUTO_TEST_CASE(test_MakeRootTreeWriterSpec)
{
  // setup the spec helper and retrieve the spec by calling the operator
  MakeRootTreeWriterSpec("writer-process",                                                          //
                         BranchDefinition<int>{InputSpec{"input1", "TST", "INTDATA"}, "intbranch"}, //
                         BranchDefinition<float>{InputSpec{"input2", "TST", "FLOATDATA"},           //
                                                 "floatbranch", "floatbranchname"}                  //
                         )();
}

template <typename T>
using Trait = RootTreeWriter::StructureElementTypeTrait<T>;
template <typename T>
using BinaryBranchStoreType = RootTreeWriter::BinaryBranchStoreType<T>;
template <typename T>
using VectorBranchStoreType = RootTreeWriter::VectorBranchStoreType<T>;
BOOST_AUTO_TEST_CASE(test_RootTreeWriterSpec_store_types)
{
  using TriviallyCopyable = o2::test::TriviallyCopyable;
  using Polymorphic = o2::test::Polymorphic;

  // simple fundamental type
  // type itself used as store type
  static_assert(std::is_same<Trait<int>::type, int>::value == true);

  // messageable type with or without ROOT dictionary
  // type itself used as store type
  static_assert(std::is_same<Trait<TriviallyCopyable>::type, TriviallyCopyable>::value == true);

  // non-messageable type with ROOT dictionary
  // pointer type used as store type
  static_assert(std::is_same<Trait<Polymorphic>::type, Polymorphic*>::value == true);

  // binary branch indicated through const char*
  // BinaryBranchStoreType is used
  static_assert(std::is_same<Trait<const char*>::type, BinaryBranchStoreType<char>>::value == true);

  // vectors of fundamental types
  // VectorBranchStoreType is used
  static_assert(std::is_same<Trait<std::vector<int>>::type, VectorBranchStoreType<std::vector<int>>>::value == true);

  // vector of messageable type with or without ROOT dictionary
  // VectorBranchStoreType is used
  static_assert(std::is_same<Trait<std::vector<TriviallyCopyable>>::type, VectorBranchStoreType<std::vector<TriviallyCopyable>>>::value == true);

  // vector of non-messageable type with ROOT dictionary
  // pointer type used as store type
  static_assert(std::is_same<Trait<std::vector<Polymorphic>>::type, std::vector<Polymorphic>*>::value == true);
}
