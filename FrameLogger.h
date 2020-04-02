#pragma once
#include <tuple>
#include <vector>
#include <fstream>

template <typename... Ts> class FrameLogger {
	// class allows logging variable number arguments per frame to txt outputfile
	// additionally, integer indices can be stored to associated frames with start of trial
	// templated types indicate the type of information that is stored, per frame, via add_frame(...)
	// can only log out types T that have (std::ofstream << T) defined; relies on C++17 (fold expression)
	// maximum number of indices and frames must be provided during construction, must not be exceeded
	// use add_index to store a frames index as trial start
	// use add_frame to store per-frame information
	// use write() to write internal information to output file; if object is destroyed without call to write(), it will try to write before destruction
	// use add_line() to write a line directly to output file, newline is appended at end
	// use add_word() to write something to output file, without newline at end
private:
	std::vector<int> m_index;
	std::vector<std::tuple<Ts...>> m_content;

	int last_index;
	int last_frame;

	bool log_written;
	std::ofstream outfile;

	std::string headline;

	template<typename... Ts>
	void write_tuple_impl(const std::tuple<Ts...>& tuple)
	{
		// print out single tuple with fold expression
		std::apply([&](const auto&... item)
			{((outfile << "\t" << item), ...); }, tuple);
	}

public:
	FrameLogger() = delete;
	FrameLogger(int num_trials, int num_frames, const char* file_name) : m_index(num_trials), m_content(num_frames), last_index(0), last_frame(0), log_written(false), headline("") {
		outfile.open(file_name, std::ios::out);
	}
	~FrameLogger() {
		if (!log_written)
			write();

		outfile.close();
	}

	void add_headline(std::string headline_in) {
		// add header to data table
		headline = headline_in;
	}

	void start_new_trial() {
		// next added frame is counted as trial start, included in m_index
		m_index[last_index] = last_frame;
		++last_index;
	}

	void add_frame(Ts... info_in) {
		m_content[last_frame] = std::tuple<Ts...>(info_in...);
		++last_frame;
	}

	template <typename... Us> void add_line(const Us&... ts) {
		(outfile << ... << ts) << std::endl;
	}

	template <typename... Us> void add_word(const Us&... ts) {
		(outfile << ... << ts);
	}

	void write() {
		// print out indices
		// iterate over m_content, printing it via _impl
		outfile << "trial index:" << std::endl;
		for (int ri = 0; ri < last_index; ++ri) {
			outfile << m_index[ri] << "\t";
		}
		outfile << std::endl;
		outfile << std::endl;

		outfile << "fr_nr\t" << headline << std::endl;
		for (int i = 0; i < last_frame; ++i) {
			outfile << i;
			write_tuple_impl(m_content[i]);
			outfile << std::endl;
		}

		log_written = true;
	}
};