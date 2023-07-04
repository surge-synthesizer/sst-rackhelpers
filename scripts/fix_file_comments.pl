#!/usr/bin/perl


use File::Find;
use File::Basename;

find(
    {
        wanted => \&findfiles,
    },
    'include'
);



sub findfiles
{

    $header = <<EOH;
/*
 * sst-rackhelpers - a Surge Synth Team product
 *
 * A set of header-only utilities we use when making stuff for VCV Rack
 *
 * Copyright 2019 - 2023, Various authors, as described in the github
 * transaction log.
 *
 * sst-rackhelpers is released under the MIT license, found in the file 
 * "LICENSE.md" in this repository.
 *
 * All source for sst-rackhelpers is available at
 * https://github.com/surge-synthesizer/sst-rackhelpers
 */
EOH

    $f = $File::Find::name;
    if ($f =~ m/\.h$/ or $f =~ m/.cpp$/)
    {
        #To search the files inside the directories
        print "Processing $f\n";

        $q = basename($f);
        print "$q\n";
        open(IN, "<$q") || die "Cant open IN $!";
        open(OUT, "> ${q}.bak") || die "Cant open BAK $!";

        $nonBlank = 0;
        $inComment = 0;
        while(<IN>)
        {
            if ($nonBlank)
            {
                print OUT
            }
            else
            {
                if (m:^\s*/\*:) {
                    $inComment = 1;
                }
                elsif (m:\s*\*/:)
                {
                    print OUT $header;
                    $nonBlank = true;
                    $inComment = false;
                }
                elsif ($inComment)
                {

                }
                elsif (m:^//:)
                {

                }
                else
                {
                    print OUT $header;
                    $nonBlank = true;
                    print OUT;

                }
            }
        }
        close(IN);
        close(OUT);
        system("mv ${q}.bak ${q}");
        system("clang-format -i ${q}");
    }
}
