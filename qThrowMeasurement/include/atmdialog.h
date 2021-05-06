#ifndef ATMDIALOG_H
#define ATMDIALOG_H

#include <ui_ATMDialog.h>

#include "envelopeextractor.h"

class ccMainAppInterface;
class ccPolyline;

class ATMDialog : public QDialog, public Ui::ATMDialog
{
    Q_OBJECT

public:
    ATMDialog(ccPolyline* polyline, ccMainAppInterface* app);
    //~ATMDialog();

    //! Loads parameters from persistent settings
    void loadParamsFromPersistentSettings();
    //! Saves parameters to persistent settings
    void saveParamsToPersistentSettings();

	//! Sets the path legnth
	void setPathLength(double l);

	//! Sets the generation step
	void setGenerationStep(double s);
	//! Sets he sections width
	void setSectionsWidth(double w);

	//! Returns the generation step
	double getGenerationStep() const;
	//! Returns the sections width
	double getSectionsWidth() const;

protected:
	void onStepChanged(double);

protected:

	//! Path length
	double m_pathLength;
	ccMainAppInterface* app;

public:

	//! Sets the number of active section(s)
	void setActiveSectionCount(int count);

	//! Sets the default section thickness
	void setSectionThickness(double t);
	//! Returns the section thickness
	double getSectionThickness() const;

	//! Returns the max edge length (for envelope generation)
	double getMaxEdgeLength() const;
	//! Sets the max edge length (for envelope generation)
	void setMaxEdgeLength(double l);

	//! Returns the envelope type (for envelope generation)
	EnvelopeExtractor::EnvelopeType getEnvelopeType() const;

	//! Whether to generate clouds or not
	bool extractClouds() const;
	//! Sets whether to generate clouds or not
	void doExtractClouds(bool state);
	//! Whether to generate envelopes or not
	bool extractEnvelopes() const;
	//! Sets whether to generate envelopes or not
	void doExtractEnvelopes(bool state, EnvelopeExtractor::EnvelopeType type);

	//! Whether to split the envelopes or not
	bool splitEnvelopes() const;
	//! Sets whether to split the envelopes or not
	void doSplitEnvelopes(bool state);

	//! Whether to use multipass or not
	bool useMultiPass() const;
	//! Sets whether to use multipass or not
	void doUseMultiPass(bool state);


	//! Whether visual debug mode is enabled or not
	bool visualDebugMode() const;
/*
private:
    Ui::ATMDialog *ui;
*/

};

#endif // ATMDIALOG_H
