#pragma once

#include <string>
#include <elfio/elfio.hpp>

// returns string contents of .bbversion section
// from EFL file or empty string if section is missing
static std::string readBuildInfo(const std::string &fname = "/proc/self/exe") {
	ELFIO::elfio elf;
	
	if (elf.load(fname)) {
		const auto *s = elf.sections[".bbversion"];
		if (s)
			return std::string(s->get_data(), s->get_size());
	}

	return "";
}
