#include <cstdlib>  // EXIT_SUCCESS

#include <gunrock/applications/color/color.hxx>

using namespace gunrock;

void test_color(int num_arguments, char** argument_array) {
  using vertex_t = int;
  using edge_t = int;
  using weight_t = float;

  constexpr memory::memory_space_t space = memory::memory_space_t::device;

  if (num_arguments != 2) {
    std::cerr << "usage: ./bin/color filename.mtx" << std::endl;
    exit(1);
  }

  // Load Matrix-Market file & convert the resultant COO into CSR format.
  std::string filename = argument_array[1];
  io::matrix_market_t<vertex_t, edge_t, weight_t> mm;
  auto coo = mm.load(filename);
  format::csr_t<memory::memory_space_t::host, vertex_t, edge_t, weight_t> csr;
  csr = coo;

  // Move data to device.
  thrust::device_vector<edge_t> d_Ap = csr.row_offsets;
  thrust::device_vector<vertex_t> d_Aj = csr.column_indices;
  thrust::device_vector<weight_t> d_Ax = csr.nonzero_values;

  thrust::device_vector<vertex_t> d_colors(csr.number_of_rows);

  // calling color
  float elapsed =
      color::execute<space>(csr.number_of_rows,      // number of vertices
                            csr.number_of_columns,   // number of columns
                            csr.number_of_nonzeros,  // number of edges
                            d_Ap,                    // row_offsets
                            d_Aj,                    // column_indices
                            d_Ax,                    // nonzero values
                            d_colors                 // output colors
      );

  std::cout << "Colors (output) = ";
  thrust::copy(d_colors.begin(), d_colors.end(),
               std::ostream_iterator<vertex_t>(std::cout, " "));
  std::cout << std::endl;

  std::cout << "color Elapsed Time: " << elapsed << " (ms)" << std::endl;
}

int main(int argc, char** argv) {
  test_color(argc, argv);
  return EXIT_SUCCESS;
}