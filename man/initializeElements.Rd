% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/initializeElements.R
\name{initializeElements}
\alias{initializeElements}
\alias{initializeCHNOPS}
\alias{initializeCHNOPSMgKCaFe}
\alias{initializeCHNOPSNaK}
\alias{initializePSE}
\alias{initializeCharges}
\alias{.getElement}
\title{Initialize (a subset of) elements of the periodic system of elements (PSE)}
\usage{
initializeElements(names)

.getElement(name, elements = NULL)

initializeCHNOPS()

initializeCHNOPSMgKCaFe()

initializeCHNOPSNaK()

initializePSE()

initializeCharges()
}
\arguments{
\item{names}{Vector of element names within PSE.}

\item{name}{Chemical element name (abbr.).}

\item{elements}{Character vector of chemical element names.}
}
\value{
A list with the elements `name` repeated sum formula, `mass` nominal 
    mass of molecule, `isotope` a list of isotopes. The function `initializeCharges` 
    is special, since it allows to parse charges as shown in examples.
}
\description{
Initialize the information about name, mass and isotopes. To 
    reduce the number of decomposition hypotheses, subsets of elements can be 
    created.
}
\details{
These functions return full, pre-defined or user-defined 
    (sub-) lists of elements.
}
\examples{
initializeCHNOPS()
getMolecule("H3O+", elements=c(initializeCHNOPS(),initializeCharges()))

}
\references{
For a description of the underlying IMS see citation("Rdisop").
    Isotope patterns were obtained through wikipedia.org
}
\author{
Steffen Neumann <sneumann@IPB-Halle.DE>
}
