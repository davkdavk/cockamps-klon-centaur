# Third-Party DSP Notices

This plugin's DSP includes modified source from the following projects:

- **JUCE 9** by ROLI/JUCE team. JUCE is dual-licensed under AGPLv3 or a commercial JUCE license; this project uses the AGPLv3 terms. See `JUCE-LICENSE.md` and the JUCE submodule's full license and third-party notices.
- **Klon Centaur Model / ChowCentaur** by Jatin Chowdhury, source revision `f3bb633a593b6fbb22a44c1ef9d1dbedbfe92d5b`. The circuit-stage source in `Source/ThirdParty/ChowCentaur/` is BSD-3-Clause; its license text is in `ChowCentaur-BSD-3-Clause.txt`. `GainStageProc` is modified to bind to this plugin's existing Drive parameter.
- **chowdsp_utils** by Chowdhury DSP, pinned at `4830738ff7e2b7e0def7036fa3b0d8f75e2983f8` as `dependencies/chowdsp_utils`. Its WDF, diode, and DSP utility code is GPLv3; the license text is in `chowdsp_utils-GPL-3.0.txt` and in the submodule.
- **TOMS Algorithm 917** by Piers Lawrence, Robert Corless, David Jeffrey, and John Burkardt. The included `toms917` source identifies itself as GNU LGPL; its license text is in `toms917-LGPL-3.0.txt`.

This project is distributed under AGPL-3.0-only. Its GPLv3 ChowDSP component is included under the compatibility terms in AGPLv3 section 13. It is not affiliated with or endorsed by the third-party authors or the Klon trademark owner.
