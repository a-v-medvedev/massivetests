#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <map>

namespace fs = std::filesystem;
using tuple_t = std::vector<std::tuple<std::string, std::string, std::string, std::string>>;

struct Config {
    std::string application = "teststub";
    int nqueued = 4;
    int repeats = 1;
//    int repeats = 3;
    std::vector<std::string> pconfs = {"2:1", "4:1"};
    std::vector<std::string> workparts = {"simple", "generating", "waiting"};
    std::string workpart_param = "3";
    std::vector<std::string> workloads = {"xxx", "yyy"};
    std::vector<std::string> confs = {"X"};
    std::vector<std::string> sections = {"sec1"};
    std::vector<std::string> parameters = {"p1"};
    std::vector<std::string> modes = {"A"};
    std::vector<std::string> submodes = {"rand50"};
//    std::vector<std::string> modes = {"A", "C", "E"};
//    std::vector<std::string> submodes = {"always", "rand50", "never"};
    bool omit_execution = false;
};

void fatal(const std::string& msg) {
    std::cerr << "FATAL: " << msg << std::endl;
    std::exit(1);
}

void info(const std::string& msg) {
    std::cout << msg << std::endl;
}

std::vector<std::string> split(const std::string& s, char delim = ' ') {
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        if (!item.empty()) elems.push_back(item);
    }
    return elems;
}

std::string join(const std::vector<std::string>& list, const std::string& delim) {
    std::ostringstream joined;
    for (size_t i = 0; i < list.size(); ++i) {
        if (i != 0) joined << delim;
        joined << list[i];
    }
    return joined.str();
}

bool are_there_files(const std::string& pattern) {
    std::string directory=".", real_pattern;
    auto path_elems = split(pattern, '/');
    real_pattern = path_elems.back();
    path_elems.pop_back();
    if (path_elems.size() > 0) {
        directory = join(path_elems, "/");
        if (pattern[0] == '/')
            directory = std::string("./") + directory;
    }
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.path().filename().string().find(real_pattern) != std::string::npos)
            return true;
    }
    return false;
}

std::vector<std::string> zip(const std::vector<std::string>& left, const std::vector<std::string>& right, const std::string& delim_local = ":") {
    std::vector<std::string> result;
    if (left.size() == 1) {
        for (const auto& r : right) result.push_back(left[0] + delim_local + r);
    } else if (right.size() == 1) {
        for (const auto& l : left) result.push_back(l + delim_local + right[0]);
    } else if (left.size() == right.size()) {
        for (size_t i = 0; i < left.size(); ++i) result.push_back(left[i] + delim_local + right[i]);
    } else {
        fatal("zip: lists of different size");
    }
    return result;
}

tuple_t
comb4(const std::vector<std::string>& A, const std::vector<std::string>& B, 
      const std::vector<std::string>& C, const std::vector<std::string>& D) {
    tuple_t result;
    for (const auto& a : A) {
        for (const auto& b : B) {
            for (const auto& c : C) {
                for (const auto& d : D) {
                    result.push_back(tuple_t::value_type(a, b, c, d));
                }
            }
        }
    }
    return result;
}

std::string to_string(tuple_t::value_type x) {
    std::string result;
    result += std::get<0>(x) + ":";
    result += std::get<1>(x) + ":";
    result += std::get<2>(x) + ":";
    result += std::get<3>(x);
    return result;
}

std::vector<std::string> comb(const std::vector<std::string>& left, const std::vector<std::string>& right, const std::string& delim_local = ":") {
    std::vector<std::string> result;
    for (const auto& l : left) {
        for (const auto& r : right) {
            result.push_back(l + delim_local + r);
        }
    }
    return result;
}

void move_results(const std::string& dir) {
    fs::remove_all(dir);
    fs::create_directory(dir);
    for (const auto& file : fs::directory_iterator(".")) {
        const auto& name = file.path().filename().string();
        if (name.find("results.") == 0) {
            fs::rename(file.path(), dir + "/" + name);
        } else if (name.find("input") == 0 && name.ends_with(".yaml")) {
            fs::copy_file(file.path(), dir + "/" + name);
        }
    }
    if (fs::exists("output_initial.yaml")) {
        fs::rename("output_initial.yaml", dir + "/output.yaml");
    }
}

void init_env() {
    if (!fs::exists("psubmit.bin")) fatal("psubmit.bin directory or symlink required.");
    if (!fs::exists("thirdparty")) fatal("thirdparty directory or symlink required.");
    if (!fs::exists("massivetest")) fatal("massivetest executable is required.");
    if (!fs::exists("script-postproc.sh")) fatal("script-postproc.sh executable is required.");
    if (!fs::exists("make_table.sh")) fatal("make_table.sh executable is required.");
}

void run_massivetest(const Config& config) {
    auto workparts = zip(config.workparts, std::vector<std::string>(config.workparts.size(), config.workpart_param));
    auto workloads = comb(config.workloads, config.confs);
    auto params = comb(config.sections, config.parameters);

    std::string command = "./massivetest --scale=" + join(config.pconfs, ",");
    command += " --nqueued=" + std::to_string(config.nqueued);
    command += " --workparts=" + join(workparts, ",");
    command += " --repeats=" + std::to_string(config.repeats);
    command += " --workloads=" + join(workloads, ",");
    command += " --parameters=" + join(params, ",");
    std::system(command.c_str());
}

void extract(const Config &config, const std::string& mode, const std::string& submode) {
    std::string conf = "conf." + mode + "_" + submode;
    std::string dir = "run/" + conf;
    std::string out_file = "run/out." + conf;

    if (!fs::exists(dir)) fatal("Missing directory: " + dir);
    if (!fs::exists(dir + "/output.yaml")) fatal("Missing output.yaml in: " + dir);

    std::ofstream out(out_file);
    if (!out) fatal("Cannot open output file: " + out_file);

    auto param_tuples = comb4(config.workloads, config.confs, config.sections, config.parameters);
    for (const auto& param : param_tuples) {
        out << to_string(param) << ":\n---\n";

        for (const auto& pconf_raw : config.pconfs) {
            std::string pconf = pconf_raw;
            std::replace(pconf.begin(), pconf.end(), ':', '/');

            for (const auto& wrpt : config.workparts) {
                std::ifstream yaml_file(dir + "/output.yaml");
                std::string line;
                std::regex pconf_match("pconf:\\s*" + pconf);
                std::regex workpart_match("Workpart:\\s*" + wrpt);
                std::regex workload_match("Workload:\\s*" + std::get<0>(param));
                std::regex conf_match("Conf:\\s*" + std::get<1>(param));

                bool found = false;
                while (std::getline(yaml_file, line)) {
                    if (!std::regex_search(line, pconf_match)) continue;
                    if (!std::regex_search(line, workpart_match)) continue;
                    if (!std::regex_search(line, workload_match)) continue;
                    if (!std::regex_search(line, conf_match)) continue;

                    std::string val = "-";
                    std::smatch mval;
                    if (std::regex_search(line, mval, std::regex("Value\\s*[:=]\\s*([^\\s,}]+)")))
                        val = mval[1];

                    std::string target_dir = "-";
                    std::smatch mdir;
                    if (std::regex_search(line, mdir, std::regex("dir\\s*[:=\\\"]+([^,\\s;)\"}]+)")))
                        target_dir = dir + "/" + std::string(mdir[1]);

                    std::string ref = "-";
                    if (target_dir != "-") {
                        std::string status;
                        if (val == "") {
                            std::string msg = std::string("ERROR: No data for {n=") + pconf + ",wprt=" + wrpt + "} in output.yaml file in the directory for extraction (" + dir + ")";
                            fatal(msg);            
                        }
                        if (val == "F") status = "FAILED";
                        else if (val == "N") status = "NONZERO";
                        else if (val == "T") status = "TIMEOUT";
                        else if (val == "A") status = "ASSERT";
                        else if (val == "C") status = "CRASH";
                        else if (val == "E") status = "EXCEPTION";

                        std::ifstream ref_in("references.txt");
                        int ref_count = std::count(std::istreambuf_iterator<char>(ref_in),
                                                   std::istreambuf_iterator<char>(), '\n');
                        ref_in.close();
                        int ref_id = ref_count + 1;

                        std::string name = fs::path(target_dir).filename().string();
                        std::string parent = fs::path(target_dir).parent_path().string();

                        std::ofstream ref_out("references.txt", std::ios::app);
                        ref_out << ref_id << ") " << status << ": " << name << "/  ---  " << parent << "  ---  " << line << "\n";

                        fs::create_directories("summary");
                        fs::copy(target_dir, "summary/" + name, fs::copy_options::recursive | fs::copy_options::overwrite_existing);

                        ref = std::to_string(ref_id);
                    }

                    out << pconf << " " << wrpt << " " << val << " " << ref << "\n";
                    found = true;
                    break;
                }
            }
        }

        out << "---\n";
    }
}

int main() {
    Config config;
    init_env();
    if (!config.omit_execution) {
        fs::remove("references.txt");
        fs::remove_all("summary");
        fs::remove_all("run");
        fs::create_directory("summary");
        fs::create_directory("run");
    }
    std::vector<std::string> combinations;
    for (const auto &t : comb4(config.workloads, config.confs, config.sections, config.parameters)) {
        combinations.push_back(to_string(t));
    }
    for (const auto& mode : config.modes) {
        for (const auto& submode : config.submodes) {
            std::string dir = "run/conf." + mode + "_" + submode;
            setenv("MASSIVE_TESTS_TESTITEM_MODE", mode.c_str(), 1);
            setenv("MASSIVE_TESTS_TESTITEM_SUBMODE", submode.c_str(), 1);
            if (!config.omit_execution) {
                run_massivetest(config);
                move_results(dir);
            }
            extract(config, mode, submode);
        }
        auto step = config.confs.size() * config.workparts.size() + 3;
        auto cmdline = std::string("./script-postproc.sh ");
        cmdline += "conf." + mode + " ";
        cmdline += std::to_string(step) + " ";
        cmdline += "\"" + join(combinations, " ") + "\"";
        std::system(cmdline.c_str());
    }
    auto cmdline = std::string("./make_table.sh ");
    cmdline += "\"" + join(combinations, " ") + "\"";
    std::system(cmdline.c_str());
	if (!are_there_files("stats.txt")) fatal("No stats.txt.");
	if (!are_there_files("run/table.")) fatal("No run/table.*.");

	for (const auto& entry : fs::directory_iterator("run")) {
		const auto& name = entry.path().filename().string();
		if (name.find("table.") == 0) {
            fs::copy_file(entry.path(), "summary/" + name, fs::copy_options::overwrite_existing);
        }
    }
	for (const auto& entry : fs::directory_iterator(".")) {
		const auto& name = entry.path().filename().string();
		if (name == "stats.txt" || name == "references.txt") {
			fs::copy_file(entry.path(), "summary/" + name, fs::copy_options::overwrite_existing);
		}
		if (name.find("test_items") == 0 && name.ends_with("yaml")) {
			fs::copy_file(entry.path(), "summary/" + name, fs::copy_options::overwrite_existing);
		}
		if (name.find("input_") == 0 && name.ends_with(".yaml")) {
			fs::copy_file(entry.path(), "summary/" + name, fs::copy_options::overwrite_existing);
		}
	}
    return 0;
} 
