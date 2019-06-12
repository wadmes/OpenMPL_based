/*************************************************************************
    > File Name: GdsiiIO.cpp
    > Author: Yibo Lin
    > Mail: yibolin@utexas.edu
    > Created Time: Wed 26 Aug 2015 10:59:58 AM CDT
 ************************************************************************/

#include "GdsiiIO.h"

SIMPLEMPL_BEGIN_NAMESPACE

namespace gtl = boost::polygon;

bool GdsReader::operator() (std::string const& filename)  
{
    // calculate file size 
    std::ifstream in (filename.c_str());
    if (!in.good()) return false;
    std::streampos begin = in.tellg();
    in.seekg(0, std::ios::end);
    std::streampos end = in.tellg();
    file_size = (end-begin);
    in.close();
    // read gds 
    return GdsParser::read(*this, filename);
}

void GdsReader::bit_array_cbk(GdsParser::GdsRecords::EnumType record_type, GdsParser::GdsData::EnumType data_type, std::vector<int> const& vBitArray)
{
    this->integer_cbk(record_type, data_type, vBitArray);
}
void GdsReader::integer_2_cbk(GdsParser::GdsRecords::EnumType record_type, GdsParser::GdsData::EnumType data_type, std::vector<int> const& vInteger)
{
    this->integer_cbk(record_type, data_type, vInteger);
}
void GdsReader::integer_4_cbk(GdsParser::GdsRecords::EnumType record_type, GdsParser::GdsData::EnumType data_type, std::vector<int> const& vInteger)
{
    this->integer_cbk(record_type, data_type, vInteger);
}
void GdsReader::real_4_cbk(GdsParser::GdsRecords::EnumType record_type, GdsParser::GdsData::EnumType data_type, std::vector<double> const& vFloat) 
{
    this->float_cbk(record_type, data_type, vFloat);
}
void GdsReader::real_8_cbk(GdsParser::GdsRecords::EnumType record_type, GdsParser::GdsData::EnumType data_type, std::vector<double> const& vFloat) 
{
    this->float_cbk(record_type, data_type, vFloat);
}
void GdsReader::string_cbk(GdsParser::GdsRecords::EnumType record_type, GdsParser::GdsData::EnumType data_type, std::string const& str) 
{
    mplAssert(data_type == GdsParser::GdsData::STRING);
    switch (record_type)
    {
        case GdsParser::GdsRecords::STRNAME:
			db.strname.assign(str);
            break;
        case GdsParser::GdsRecords::LIBNAME:
        case GdsParser::GdsRecords::STRING:
        default: break;
    }
}
void GdsReader::begin_end_cbk(GdsParser::GdsRecords::EnumType record_type)
{
    switch (record_type)
    {
        case GdsParser::GdsRecords::BOX:
        case GdsParser::GdsRecords::BOUNDARY:
        case GdsParser::GdsRecords::PATH:
            vPoint.clear();
            layer = 0;
            status = record_type;
            break;
        case GdsParser::GdsRecords::ENDEL:
            {
                switch (status)
                {
                    case GdsParser::GdsRecords::BOX:
                    case GdsParser::GdsRecords::BOUNDARY:
                    case GdsParser::GdsRecords::PATH:
                        mplAssert(layer != -1);
                        db.add(layer, vPoint);
                        break;
                    default: break;
                }
                status = GdsParser::GdsRecords::UNKNOWN;
            }
            break;
        case GdsParser::GdsRecords::ENDLIB: // notify database on the end of lib 
            db.end_lib();
            break;
        case GdsParser::GdsRecords::ENDSTR: // currently not interested, add stuff here if needed 
            db.end_str();
            break;
        default: // be careful here, you may dump a lot of unnecessary error message for unknown record_type 
            mplPrint(kERROR, "%s() unsupported record_type = %s", __func__, GdsParser::gds_record_ascii(record_type));
            break;
    }
}
/// helper functions 
void GdsReader::integer_cbk(GdsParser::GdsRecords::EnumType record_type, GdsParser::GdsData::EnumType /*data_type*/, std::vector<int> const& vData)
{
    switch (record_type)
    {
        case GdsParser::GdsRecords::LAYER:
            layer = vData[0];
            break;
        case GdsParser::GdsRecords::XY:
            if (status == GdsParser::GdsRecords::BOX || status == GdsParser::GdsRecords::BOUNDARY || status == GdsParser::GdsRecords::PATH)
            {
				mplAssert((vData.size() & 1) == 0 && vData.size() >= 4);
				vPoint.clear();
				uint32_t end = vData.size();
				// skip last point for BOX and BOUNDARY
				if (status == GdsParser::GdsRecords::BOX || status == GdsParser::GdsRecords::BOUNDARY) end -= 2;
				for (uint32_t i = 0; i < end; i += 2)
					vPoint.push_back(gtl::construct<point_type>(vData[i], vData[i+1]));
            }
            break;
        case GdsParser::GdsRecords::BGNLIB: // notify database on the begin of lib 
            db.begin_lib(); 
            break;
        case GdsParser::GdsRecords::BGNSTR: // just date of creation, not interesting
            db.begin_str();
            break;
        default: // other not interested record_type
            //mplPrint(kERROR, "%s() invalid record_type = %s, data_type = %s", __func__, GdsParser::gds_record_ascii(record_type), GdsParser::gds_data_ascii(data_type));
            break;
    }
}
void GdsReader::float_cbk(GdsParser::GdsRecords::EnumType record_type, GdsParser::GdsData::EnumType data_type, std::vector<double> const& vData)
{
    switch (record_type)
    {
        case GdsParser::GdsRecords::UNITS:
			db.unit = vData[1]; 
            break;
        default:
            mplPrint(kERROR, "%s() invalid record_type = %s, data_type = %s", __func__, GdsParser::gds_record_ascii(record_type), GdsParser::gds_data_ascii(data_type));
            break;
    }
}

void GdsWriter::operator() (std::string const& filename, GdsWriter::layoutdb_type const& db, 
        std::vector<std::pair<uint32_t, uint32_t> > const& vConflict, 
        std::vector<std::vector<uint32_t> > const& mAdjVertex, 
        std::string const& strname, double unit) const 
{
    GdsParser::GdsWriter gw (filename.c_str());
    gw.gds_create_lib("POLYGONS", unit /* um per bit */ );
    gw.gds_write_bgnstr();
    gw.gds_write_strname(strname.c_str());

    // if there are precolored patterns, keep the same layer convention 
    int32_t layer_offset = (db.parms.sPrecolorLayer.empty())? 100 : *db.parms.sPrecolorLayer.begin();
    // basic operation
    // will add more 
    write_rectangles(gw, db.polyrect_patterns(), layer_offset);
    write_conflicts(gw, db, vConflict,  layer_offset+db.color_num());   // conflict layer 
    write_edges(gw, db, mAdjVertex, layer_offset+db.color_num()+1); // draw edges 
    //(*this)(gw, db.hPath); // draw edges if there exits 

    gw.gds_write_endstr();
    gw.gds_write_endlib(); 
}

void GdsWriter::operator() (std::string const& filename, layoutdb_type const& db,
	std::vector<std::pair<uint32_t, uint32_t> > const& vConflict,
	std::vector<std::vector<uint32_t> > const& vStitches,
	std::vector<std::vector<uint32_t> > const& mAdjVertex) const
{
	GdsParser::GdsWriter gw(filename.c_str());
	gw.gds_create_lib("POLYGONS", db.unit*1e+9 /* um per bit */);
	gw.gds_write_bgnstr();
	gw.gds_write_strname(db.strname.c_str());

	// if there are precolored patterns, keep the same layer convention 
	int32_t layer_offset = (db.parms.sPrecolorLayer.empty()) ? 100 : *db.parms.sPrecolorLayer.begin();
	// basic operation
	// will add more 
	write_rectangles(gw, db.polyrect_patterns(), layer_offset);

	write_conflicts(gw, db, vConflict, layer_offset + db.color_num());   // conflict layer 
	write_edges(gw, db, vStitches, layer_offset + db.color_num() + 1);  // draw stitches
	write_edges(gw, db, mAdjVertex, layer_offset + db.color_num() + 2); // draw edges 

	gw.gds_write_endstr();
	gw.gds_write_endlib();
}

void GdsWriter::write_rectangles(GdsParser::GdsWriter& gw, std::vector<GdsWriter::rectangle_pointer_type> const& vRect, const int32_t layer_offset) const 
{
    for (std::vector<rectangle_pointer_type>::const_iterator it = vRect.begin(); it != vRect.end(); ++it)
    {
        rectangle_type const& rect = **it;
        gw.write_box(layer_offset+rect.color(), 0, 
                gtl::xl(rect), gtl::yl(rect), 
                gtl::xh(rect), gtl::yh(rect));
#ifdef DEBUG
        mplAssert(rect.color() >= 0);
#endif
    }
}
void GdsWriter::write_conflicts(GdsParser::GdsWriter& gw, GdsWriter::layoutdb_type const& db, 
        std::vector<std::pair<uint32_t, uint32_t> > const& vConflict, const int32_t layer) const
{
    for (std::vector<std::pair<uint32_t, uint32_t> >::const_iterator it = vConflict.begin(); it != vConflict.end(); ++it)
    {
        // create a path
        gw.gds_write_path();
        gw.gds_write_layer(layer);
        gw.gds_write_datatype(0);
        gw.gds_write_pathtype(2); // extended square ends
        gw.gds_write_width(5); // 5 nm wide

        point_type vc = db.get_point_closest_to_center(it->first);
        point_type uc = db.get_point_closest_to_center(it->second);
        int32_t x[2] = {gtl::x(vc), gtl::x(uc)};
        int32_t y[2] = {gtl::y(vc), gtl::y(uc)};

        gw.gds_write_xy(x, y, 2);
        gw.gds_write_endel();
    }
}
void GdsWriter::write_paths(GdsParser::GdsWriter& gw, std::map<int32_t, std::vector<GdsWriter::path_type> > const& hPath) const 
{
    for (std::map<int32_t, std::vector<path_type> >::const_iterator it1 = hPath.begin(); it1 != hPath.end(); ++it1)
    {
        const int32_t layer = it1->first;
        std::vector<path_type> const& vPath = it1->second;
        for (std::vector<path_type>::const_iterator it2 = vPath.begin(); it2 != vPath.end(); ++it2)
        {
            path_type const& path = *it2;
            // create a path
            gw.gds_write_path();
            gw.gds_write_layer(layer);
            gw.gds_write_datatype(0);
            gw.gds_write_pathtype(2); // extended square ends
            gw.gds_write_width(5); // 5 nm wide

            int32_t x[2] = {gtl::x(gtl::low(path)), gtl::x(gtl::high(path))};
            int32_t y[2] = {gtl::y(gtl::low(path)), gtl::y(gtl::high(path))};

            gw.gds_write_xy(x, y, 2);
            gw.gds_write_endel();
        }
    }
}
void GdsWriter::write_edges(GdsParser::GdsWriter& gw, GdsWriter::layoutdb_type const& db, std::vector<std::vector<uint32_t> > const& mAdjVertex, const int32_t layer) const 
{
    for (uint32_t i = 0; i != mAdjVertex.size(); ++i)
    {
        for (uint32_t j = 0; j != mAdjVertex[i].size(); ++j)
        {
            uint32_t v = i;
            uint32_t u = mAdjVertex[i][j];
            // create a path from v to u 
            if (v < u) // avoid duplicate 
            {
                // create a path
                gw.gds_write_path();
                gw.gds_write_layer(layer);
                gw.gds_write_datatype(0);
                gw.gds_write_pathtype(2); // extended square ends
                gw.gds_write_width(5); // 5 nm wide

                point_type vc = db.get_point_closest_to_center(v);
                point_type uc = db.get_point_closest_to_center(u);
                int32_t x[2] = {gtl::x(vc), gtl::x(uc)};
                int32_t y[2] = {gtl::y(vc), gtl::y(uc)};

                gw.gds_write_xy(x, y, 2);
                gw.gds_write_endel();
            }
        }
    }
}

void GdsWriter::write_edges(GdsParser::GdsWriter& gw, GdsWriter::layoutdb_type const& db, std::vector<std::vector<uint32_t> > const& mAdjVertex, std::vector<bool> isVDDGND, const int32_t layer) const
{
	for (uint32_t i = 0; i != mAdjVertex.size(); ++i)
	{
		for (uint32_t j = 0; j != mAdjVertex[i].size(); ++j)
		{
			uint32_t v = i;
			uint32_t u = mAdjVertex[i][j];
			// create a path from v to u 
			if (v < u) // avoid duplicate 
			{
				// create a path
				gw.gds_write_path();
				if (isVDDGND[v] || isVDDGND[u])
					gw.gds_write_layer(layer + 1);
				else
					gw.gds_write_layer(layer);
				gw.gds_write_datatype(0);
				gw.gds_write_pathtype(2); // extended square ends
				gw.gds_write_width(5); // 5 nm wide

				point_type vc = db.get_point_closest_to_center(v);
				point_type uc = db.get_point_closest_to_center(u);
				int32_t x[2] = { gtl::x(vc), gtl::x(uc) };
				int32_t y[2] = { gtl::y(vc), gtl::y(uc) };

				gw.gds_write_xy(x, y, 2);
				gw.gds_write_endel();
			}
		}
	}
}


void GdsWriter::write_Simplification(std::string const filename, GdsWriter::layoutdb_type const& db, std::vector<uint32_t> m_vCompId, std::vector<std::vector<uint32_t> > m_mAdjVertex, std::vector<bool> in_DG, std::vector<bool> isVDDGND, bool lg) const
{
	GdsParser::GdsWriter gw(filename.c_str());
	gw.gds_create_lib("POLYGON", db.unit*1e+6);
	gw.gds_write_bgnstr();
	gw.gds_write_strname(db.strname.c_str());

	std::vector<rectangle_pointer_type> rect_vec = db.polyrect_patterns();
	std::vector<uint32_t> Poly_Rect_begin = db.PolyRectBgnLoc();				///< original polygons mapping to rectangles
	std::vector<uint32_t> Poly_Rect_end;
	uint32_t vertex_num = db.vPatternBbox.size();
	Poly_Rect_end.resize(vertex_num);

	for (uint32_t i = 0; i < vertex_num - 1; i++)
		Poly_Rect_end[i] = Poly_Rect_begin[i + 1] - 1;
	Poly_Rect_end[vertex_num - 1] = rect_vec.size() - 1;

	for (uint32_t i = 0; i < vertex_num; i++)
	{
		uint32_t start_idx = Poly_Rect_begin[i];
		uint32_t end_idx = Poly_Rect_end[i];
		uint32_t color;
		if (lg)
		{
			if (in_DG[i])
				color = m_vCompId[i] + 1;
			else
				color = 0;
		}
		else {
			color = m_vCompId[i] + 1;
		}
		for (uint32_t j = start_idx; j <= end_idx; j++)
		{
			rectangle_type const& rect = *rect_vec[j];
			gw.write_box(99 + color, 0,
				gtl::xl(rect), gtl::yl(rect),
				gtl::xh(rect), gtl::yh(rect));
		}
	}
	uint32_t edge_color = *std::max_element(m_vCompId.begin(), m_vCompId.end());
	this->write_edges(gw, db, m_mAdjVertex, isVDDGND, 99 + edge_color + 3);
	gw.gds_write_endstr();
	gw.gds_write_endlib();
}
SIMPLEMPL_END_NAMESPACE
