#ifndef IMS_PMFFRAGMENTER_H
#define IMS_PMFFRAGMENTER_H

#include <vector>

#include <ims/fragmenter.h>
#include <ims/alphabet.h>
#include <ims/functors/alphabetgetmass.h>

namespace ims {

/**
 * Computes the peptide mass fingerprint from a sequence.
 * For this, a list of cleavage characters
 * is specified. After these characters, the sequence is cut. It can be specified
 * whether the cleavage character at the end of each fragment should be inserted
 * into the fragment (tryptic digestion setting) or discarded (RNAses setting).
 *
 * @param MassType
 * @param ScaledMassType is only used as the second template parameter for Alphabet.
 * @param GetMassFunctor
 *
 * @todo Probably ScaledMassType should @b not be a template parameter, but
 *   the Alphabet should be.
 */
template <typename MassType, typename ScaledMassType, typename GetMassFunctor=AlphabetGetMassFunctor >
class PMFFragmenter : public Fragmenter<MassType> {
	public:
		typedef MassType mass_type;
		typedef typename Fragmenter<MassType>::peak_type peak_type;
		typedef typename Fragmenter<MassType>::peaklist_type peaklist_type;
		typedef Alphabet alphabet_type;
		PMFFragmenter(const alphabet_type& alphabet, const std::string& cleavage_characters,
			const std::string& prohibition_characters, bool withCleave=true);
		PMFFragmenter(const PMFFragmenter<MassType,ScaledMassType,GetMassFunctor>& fragmenter);

		virtual void predictSpectrum(peaklist_type* peaklist, const std::string& sequence);
		virtual void setMaxMiscleaves(size_t max_miscleaves) { this->max_miscleaves=max_miscleaves; }
		virtual size_t getMaxMiscleaves() { return max_miscleaves; }
	protected:
		// data structure for subfragments
		typedef struct {
			mass_type mass;
			mass_type cleavage_char_mass;
			size_t length;
			size_t cleavage_length;
			size_t start;
		} subfragment_t;

		using Fragmenter<MassType>::modifier;
		alphabet_type alphabet;
		std::string cleavage_characters;
		std::string prohibition_characters;
		bool withCleave;
		size_t max_miscleaves;
};


/**
 * Constructs peptide mass fingerprint fragmenter.
 *
 * @param alphabet Weighted alphabet for character masses.
 * @param cleavage_characters List of cleavage characters which mark ends of fragments.
 * @param prohibition_characters If a prohibition character is encountered after
 *    a cleavage character, the sequence is @b not cut at this cleavage character.
 * @param withCleave If true, the right cleavage-character is included in the fragment, otherwise it is deleted
 */
template <typename MassType, typename ScaledMassType, typename GetMassFunctor>
PMFFragmenter<MassType,ScaledMassType,GetMassFunctor>::PMFFragmenter(
		const alphabet_type& alphabet,
		const std::string& cleavage_characters,
		const std::string& prohibition_characters,
		bool withCleave
	) :
	alphabet(alphabet),
	cleavage_characters(cleavage_characters),
	prohibition_characters(prohibition_characters),
	withCleave(withCleave),
	max_miscleaves(0)
{
}

/**
 * Copy constructor.
 */
template <typename MassType, typename ScaledMassType, typename GetMassFunctor>
PMFFragmenter<MassType,ScaledMassType,GetMassFunctor>::PMFFragmenter(const PMFFragmenter<MassType,ScaledMassType,GetMassFunctor>& fragmenter) :
	Fragmenter<MassType>(fragmenter),
	alphabet(fragmenter.alphabet),
	cleavage_characters(fragmenter.cleavage_characters),
	prohibition_characters(fragmenter.prohibition_characters),
	withCleave(fragmenter.withCleave),
	max_miscleaves(0)
{
}


/**
 * Computes masses of the predicted spectrum generated by a sequence.
 * Computes fragment list of given sequence using the given cleavage scheme.
 * Fragments are stored in order of occurence in the sequence, even duplicates. If this
 * is not desired, please use a modifier (see Fragmenter::setModifier), for example
 * SortModifier or UnificationModifier.
 * @param peaklist Pointer to peaklist in which the fragment peaks are to be stored in. Peaklist
 *    is cleared before new fragments are added.
 * @param sequence The sequence for which the spectrum-masses are computed.
 */
template <typename MassType, typename ScaledMassType, typename GetMassFunctor>
void PMFFragmenter<MassType,ScaledMassType,GetMassFunctor>::predictSpectrum(peaklist_type* peaklist, const std::string& sequence) {
	// instantiate functor
	GetMassFunctor get_mass_functor;
	peaklist->clear();

	std::vector<subfragment_t> subfragments;

	// STEP 1: break into sub-fragments
	subfragment_t subfragment = {(mass_type)0.0, (mass_type)0.0, 0, 0, 0};
	for(size_t i=0; i<sequence.size(); ++i) {
		// is current char a cleavage character?
		//bool cleave_here=(cleavage_characters.find_first_of(sequence[i]) != std::string::npos);
		char c=sequence[i];
		std::string s=cleavage_characters;
		bool cleave_here=(s.find_first_of(c) != std::string::npos);
		// if so, find out if its followed by a prohibition char
		if (cleave_here) {
			if ((i+1)<sequence.size()) {
				cleave_here=(prohibition_characters.find_first_of(sequence[i+1])==std::string::npos);
			}
		}
		if (cleave_here) {
			// push back current subfragment
			subfragment.cleavage_length=1;
			subfragment.cleavage_char_mass=get_mass_functor(alphabet, std::string(1,sequence[i]));
			subfragments.push_back(subfragment);
			// initialize new subfragment
			subfragment.mass=(mass_type)0.0;
			subfragment.cleavage_char_mass=(mass_type)0.0;
			subfragment.start=i+1;
			subfragment.length=0;
			subfragment.cleavage_length=0;
		} else {
			subfragment.mass+=get_mass_functor(alphabet, std::string(1,sequence[i]));
			subfragment.length++;
		}
	}
    // check if last fragment has to be finished
	if (subfragment.length>0) {
		subfragments.push_back(subfragment);
	}

	// STEP 2: use subfragments to calculate peaklist
	typename std::vector<subfragment_t>::const_iterator it = subfragments.begin();
	for (; it!=subfragments.end(); ++it) {
		size_t length=0;
		mass_type mass = (mass_type)0.0;
		// generate every possible fragment compatible with max_miscleaves
		for (size_t i=0; i<=max_miscleaves; ++i) {
			if (it+i==subfragments.end()) break;
			length+=(it+i)->length;
			mass+=(it+i)->mass;
			peak_type peak;
			// does cleavage character belong to fragment?
			if (withCleave) {
				peak = peak_type(mass+((it+i)->cleavage_char_mass), it->start, length+((it+i)->cleavage_length));
			} else {
				peak = peak_type(mass, it->start, length);
			}
			peak.setMiscleavageCount(i);
			// omit fragments of length 0
			if (peak.getLength()>0) peaklist->push_back(peak);
			// add cleavage character, since its in the bigger fragment
			length+=(it+i)->cleavage_length;
			mass+=(it+i)->cleavage_char_mass;
		}
	}

	// STEP 3: Apply modifier
	if (modifier.get()!=0) {
		modifier->modify(*peaklist);
	}
}

}
#endif