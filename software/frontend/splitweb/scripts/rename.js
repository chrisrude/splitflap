import fs from 'fs';

const BUILD_DIR = './build';
const BUILD_DIR_SHORTENED = './build_shortened';
const IGNORE_FILES = ['index.html', 'favicon.png', '.DS_STORE', '_app/version.json'];
const MAX_PATH_LENGTH = 32;
const DRY_RUN = false;

// read all files in the build directory into a list
const files = fs.readdirSync(BUILD_DIR, { recursive: true });

// remove all files in the ignore list
IGNORE_FILES.forEach(file => {
    const index = files.indexOf(file);
    if (index > -1) {
        files.splice(index, 1);
    }
});

// make_shortname function:
// for each component of the file name (directory 
// and the part of the filename before the first '.')
// replace that string with the first letter of that string.
// if the string is only one character, leave it alone.
// if there is already a short file with that name, choose the
// next letter in the string instead.
const make_shortname = (file) => {
    const dir_renames = new Map();
    const file_renames = new Map();

    const parts = file.split('/');
    const short_parts = parts.map(part => {
        const names = part.split('.');
        if (names[0].length === 1) {
            // already short, leave it alone
            return part;
        }
        const short_name = names[0][0];

        if (names.length > 1) {
            // we have a file, put its mapping into file_renames
            names[0] = short_name;
            file_renames.set(part, names.join('.'));
        } else {
            // we have a directory, put its mapping into dir_renames
            dir_renames.set(part, short_name);
            names[0] = short_name;
        }

        return names.join('.');
    });

    // todo: look for dups

    return [short_parts.join('/'), dir_renames, file_renames];
};

// build a map of long file name to short file name
const file_renames_all = new Map();
const dir_renames_all = new Map();
const fileMap = new Map();

files.forEach(file => {
    const [shortName, dir_renames, file_renames] = make_shortname(file);
    dir_renames.forEach((value, key) => {
        dir_renames_all.set(key, value);
    });
    file_renames.forEach((value, key) => {
        file_renames_all.set(key, value);
    });

    fileMap.set(file, shortName);
});

// check if every value in fileMap is unique.  If not print
// the duplicates and exit.
const values = Array.from(fileMap.values());
const duplicates = values.filter((value, index, self) => {
    return self.indexOf(value) !== index;
});
if (duplicates.length > 0) {
    console.error('duplicates found!');
    console.error(duplicates);
}

// if any of the short names are longer than MAX_PATH_LENGTH, print
// them and exit.
const long_names = values.filter(value => {
    return value.length > MAX_PATH_LENGTH;
});
if (long_names.length > 0) {
    console.error('long short names found!');
    console.error(long_names);
}

if (long_names.length > 0 || duplicates.length > 0) {
    console.error('exiting...');
    process.exit(1);
}

const make_replacement_map = (dir_renames, file_renames) => {
    const replacement_map = new Map();
    // create replacement_map to include the patterns we will replace in each
    // file.
    // for directories, look for the pattern:
    //    /dir_renames.key()/ and replace it with /dir_renames.get()/
    // for files, look for the pattern:
    //   /file_renames.key() and replace it with /file_renames.get()
    dir_renames.forEach((value, key) => {
        replacement_map.set('/' + key + '/', '/' + value + '/');
    });
    file_renames.forEach((value, key) => {
        replacement_map.set('/' + key, '/' + value);
    });
    return replacement_map;
};

// function recursive_replace
// opens each file in the build directory and replaces all instances
// of long_string with short_string, and then writes a copy of the
// shortened file into the BUILD_DIR_SHORTENED directory.
function replace_all(base_dir, short_base_dir, file_map, replacement_map) {

    // make short_dir if it doesn't exist
    if (!fs.existsSync(short_base_dir)) {
        console.log('making dir: ' + short_base_dir);
        DRY_RUN || fs.mkdirSync(short_base_dir);
    }

    const files = fs.readdirSync(base_dir, { recursive: true });
    files.forEach(file => {
        const full_path = base_dir + '/' + file;
        const stats = fs.statSync(full_path);
        if (stats.isDirectory()) {
            // make the directory in the short dir
            const short_path = (file_map.get(file) ?? file);
            console.log('making dir:\t' + file + '\t=>\t' + short_path);
            DRY_RUN || fs.mkdirSync(short_base_dir + '/' + short_path, { recursive: true });
            return;
        }

        // only read .css and .js files
        if (!file.endsWith('.css') && !file.endsWith('.js') && !file.endsWith('.html')) {
            // just copy the file
            let short_path = file_map.get(file);
            if (undefined === short_path) {
                // get everything in file until the last '/'
                const parts = file.split('/');
                const file_only = parts.pop();
                const dir = parts.join('/');
                // translate dir
                if ('' == dir) {
                    short_path = file_only;
                } else {
                    short_path = file_map.get(dir) + '/' + file_only;
                }
            }
            console.log('copying:\t' + file + '\t=>\t' + short_path);
            DRY_RUN || fs.copyFileSync(full_path, short_base_dir + '/' + short_path);
            return;
        }

        // read, modify, and write out the file to the short dir
        let contents = fs.readFileSync(full_path, 'utf8');
        replacement_map.forEach((value, key) => {
            contents = contents.replaceAll(key, value);
        });

        const short_name = (file_map.get(file) ?? file);
        console.log('translating:\t' + file + '\t=>\t' + short_name);
        DRY_RUN || fs.writeFileSync(short_base_dir + '/' + short_name, contents);
    });
}

// recursively remove BUILD_DIR_SHORTENED if it exists
if (fs.existsSync(BUILD_DIR_SHORTENED)) {
    console.log('removing dir: ' + BUILD_DIR_SHORTENED);
    DRY_RUN || fs.rmSync(BUILD_DIR_SHORTENED, { recursive: true });
}

const replacement_map = make_replacement_map(dir_renames_all, file_renames_all);
replace_all(BUILD_DIR, BUILD_DIR_SHORTENED, fileMap, replacement_map);

console.log('done! :)');
