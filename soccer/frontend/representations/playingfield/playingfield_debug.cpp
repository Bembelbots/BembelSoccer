/**
 * Generates a svg-image from the playingfield definitions to visualize
 * which playingfield the robot is using.
 *
 * Build:
 *   cd debug
 *   make playingfield_debug
 *
 * Usage:
 *   bin/playingfield_debug <field> <target_file>
 *
 * Example:
 *  Running `bin/playingfield_debug 1 spl_field.svg` will print the definitions
 *  for SPL field to the file 'spl_field.svg'.
 */

#include "playingfield_painter.h"

#include <iomanip>
#include <iostream>
#include <boost/program_options.hpp>

#include <framework/image/svg/image.h>
#include <representations/bembelbots/types.h>
#include <representations/bembelbots/thread.h>
#include <framework/thread/simplethreadmanager.h>


using namespace std;

ThreadManager* GetThreadManager() {
    using T = SimpleThreadManager<NaoThread>;
    static ThreadManager manager(new T());
    return &manager;
}

void CreateXLoggerThread(XLogger *logger) {
    using namespace std::placeholders;
    GetThreadManager()->create(NaoThread::IO, std::bind(&XLogger::io_worker, logger, _1));
}

int main(int argc, const char *argv[]) {
    std::string fname, json;
    int field;

    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("field,f", po::value<int>(&field)->default_value(int(FieldSize::SPL)), "fieldsize")
        ("output,o", po::value<std::string>(&fname)->default_value("field.svg"), "output filename")
        ("json,j", po::value<std::string>(&json)->default_value(""), "use field_dimensions.json")
        ("crosses,c", "draw crosses")
        ("initial,i", "draw initial positions")
        ("ready,r", "draw ready positions")
        ("challenge,l", "draw challenge positions")
        ("all,a", "enable all options")
        ("help,h", "produce help message.");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << std::endl << desc << std::endl;
        
        std::cout << "FieldSize: " << std::endl;
        for (auto e: enumValues<FieldSize>)
            std::cout << std::setw(4) << static_cast<int>(e) << "\t" << enumToStr(e) << std::endl;
        std::cout << std::endl;

        return 0;
    }

    PlayingField pf(static_cast<FieldSize>(field));
    if (!json.empty())
        pf = PlayingField(json);
    
    SVGImage img;
    bool force = vm.count("all");

    PlayingfieldPainter pfp(img, pf);

    pfp.drawBackground();
    pfp.drawLines();
    pfp.drawPoles();
    if (vm.count("crosses") || force)
        pfp.drawCrosses();
    if (vm.count("initial") || force)
        pfp.drawInitialPositions();
    if (vm.count("ready") || force)
        pfp.drawReadyPositions();
    if (vm.count("challenge") || force)
        pfp.drawChallengePositions();

    img.saveToFile(fname);

    std::cout << "Output file: " << fname << std::endl;

    return 0;
}

// vim: set ts=4 sw=4 sts=4 expandtab:
